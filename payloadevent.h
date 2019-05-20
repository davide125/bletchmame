/***************************************************************************

    payloadevent.h

    Generic class for an event that takes a C++-ish payload

***************************************************************************/

#pragma once

#ifndef PAYLOADEVENT_H
#define PAYLOADEVENT_H


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template<class T>
class PayloadEvent : public wxEvent
{
public:
    PayloadEvent(wxEventType event_type, int winid, T &&payload)
        : PayloadEvent(event_type, winid, std::make_shared<T>(std::move(payload)))
    {
    }

    T &Payload() { return *m_ptr; }

protected:
    virtual wxEvent *Clone() const override
    {
        return new PayloadEvent(GetEventType(), GetId(), m_ptr);
    }

private:
    std::shared_ptr<T>  m_ptr;

    PayloadEvent(wxEventType event_type, int winid, std::shared_ptr<T> ptr)
        : wxEvent(winid, event_type)
        , m_ptr(std::move(ptr))
    {
    }
};

#endif // PAYLOADEVENT_H
