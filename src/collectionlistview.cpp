/***************************************************************************

	collectionlistview.cpp

	Subclass of wxListView, providing rich views on collections

***************************************************************************/

#include "collectionlistview.h"
#include "utility.h"
#include "prefs.h"


//-------------------------------------------------
//  GetStyle
//-------------------------------------------------

static long GetStyle(bool support_label_edit)
{
	// determine the style
	long style = wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL;
	if (support_label_edit)
		style |= wxLC_EDIT_LABELS;
	return style;
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

CollectionListView::CollectionListView(wxWindow &parent, wxWindowID winid, Preferences &prefs, const CollectionViewDesc &desc, std::unique_ptr<ICollectionImpl> &&coll_impl, bool support_label_edit)
	: wxListView(&parent, winid, wxDefaultPosition, wxDefaultSize, GetStyle(support_label_edit))
	, m_desc(desc)
	, m_prefs(prefs)
	, m_coll_impl(std::move(coll_impl))
	, m_sort_column(0)
	, m_sort_type(ColumnPrefs::sort_type::ASCENDING)
{
	// get preferences
	const std::unordered_map<std::string, ColumnPrefs> &col_prefs = m_prefs.GetColumnPrefs(desc.m_name);

	// prepare an array for order
	std::vector<int> order;
	order.resize(desc.m_columns.size());

	// build the columns
	ClearAll();
	for (int i = 0; i < desc.m_columns.size(); i++)
	{
		// defaults
		int width = desc.m_columns[i].m_default_width;
		order[i] = i;

		// look up this column
		auto iter = col_prefs.find(desc.m_columns[i].m_id);
		if (iter != col_prefs.end())
		{
			// figure out the width
			if (iter->second.m_width > 0)
				width = iter->second.m_width;

			// figure out the order
			order[i] = iter->second.m_order;

			// is there a sort?
			if (iter->second.m_sort.has_value())
			{
				m_sort_column = i;
				m_sort_type = iter->second.m_sort.value();
			}
		}

		// append the actual column
		AppendColumn(desc.m_columns[i].m_description, wxLIST_FORMAT_LEFT, width);
	}

	// set the column order
	SetColumnsOrder(wxArrayInt(order.begin(), order.end()));

	// bind events
	Bind(wxEVT_LIST_COL_END_DRAG,	[this](auto &)		{ UpdateColumnPrefs();					});
	Bind(wxEVT_LIST_COL_CLICK,		[this](auto &event)	{ ToggleColumnSort(event.GetColumn());	});
}


//-------------------------------------------------
//  UpdateMachineList
//-------------------------------------------------

void CollectionListView::UpdateListView()
{
	// identify the selsection
	long first_selected = GetFirstSelected();
	int actual_selected = first_selected >= 0 ? GetActualIndex(first_selected) : -1;

	// get basic info about things
	long collection_size = m_coll_impl->GetSize();
	int column_count = GetColumnCount();

	// rebuild the indirection list
	m_indirections.clear();
	m_indirections.reserve(collection_size);
	for (long i = 0; i < collection_size; i++)
	{
		// check for a match
		bool match = m_filter_text.empty();
		for (int column = 0; !match && (column < column_count); column++)
		{
			wxString cell_text = GetActualItemText(i, column);
			match = util::string_icontains(cell_text, m_filter_text);
		}

		if (match)
			m_indirections.push_back(i);
	}

	// sort the indirection list
	std::sort(m_indirections.begin(), m_indirections.end(), [this](int a, int b)
	{
		const wxString &a_string = GetActualItemText(a, m_sort_column);
		const wxString &b_string = GetActualItemText(b, m_sort_column);
		int compare_result = util::string_icompare(a_string, b_string);
		return m_sort_type == ColumnPrefs::sort_type::ASCENDING
			? compare_result < 0
			: compare_result > 0;
	});

	// set the list view size
	SetItemCount(m_indirections.size());
	RefreshItems(0, m_indirections.size() - 1);

	// restore the selection, if appropriate (note this will cause filtering to drop the selection if
	// it is lost; this should really be fixed)
	if (actual_selected > 0)
	{
		auto iter = std::find(m_indirections.begin(), m_indirections.end(), actual_selected);
		if (iter != m_indirections.end())
		{
			int index = iter - m_indirections.begin();
			Select(index);
			EnsureVisible(index);
		}
	}
}


//-------------------------------------------------
//  SetFilterText
//-------------------------------------------------

void CollectionListView::SetFilterText(wxString &&filter_text)
{
	if (m_filter_text != filter_text)
	{
		m_filter_text = std::move(filter_text);
		UpdateListView();
	}
}


//-------------------------------------------------
//  OnGetItemText
//-------------------------------------------------

wxString CollectionListView::OnGetItemText(long item, long column) const
{
	long actual_item = m_indirections[item];
	return GetActualItemText(actual_item, column);
}


//-------------------------------------------------
//  GetActualItemText
//-------------------------------------------------

const wxString &CollectionListView::GetActualItemText(long item, long column) const
{
	return m_coll_impl->GetItemText(item, column);
}


//-------------------------------------------------
//  UpdateColumnPrefs
//-------------------------------------------------

void CollectionListView::UpdateColumnPrefs()
{
	// get the column count
	int column_count = GetColumnCount();

	// start preparing column prefs
	std::unordered_map<std::string, ColumnPrefs> col_prefs;
	col_prefs.reserve(column_count);

	// get all info about each column
	wxArrayInt order = GetColumnsOrder();
	for (int i = 0; i < column_count; i++)
	{
		ColumnPrefs &this_col_pref = col_prefs[m_desc.m_columns[i].m_id];
		this_col_pref.m_width = GetColumnWidth(i);
		this_col_pref.m_order = order[i];
		this_col_pref.m_sort = m_sort_column == i ? m_sort_type : std::optional<ColumnPrefs::sort_type>();
	}

	// and save it
	m_prefs.SetColumnPrefs(m_desc.m_name, std::move(col_prefs));
}


//-------------------------------------------------
//  UpdateColumnPrefs
//-------------------------------------------------

void CollectionListView::ToggleColumnSort(int column_index)
{
	if (m_sort_column != column_index)
	{
		m_sort_column = column_index;
		m_sort_type = ColumnPrefs::sort_type::ASCENDING;
	}
	else
	{
		m_sort_type = m_sort_type == ColumnPrefs::sort_type::ASCENDING
			? ColumnPrefs::sort_type::DESCENDING
			: ColumnPrefs::sort_type::ASCENDING;
	}
	UpdateListView();
}
