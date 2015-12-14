#ifndef LINKEDLISTITEM_H_
#define LINKEDLISTITEM_H_

#include <Arduino.h>

template<typename T>
class LinkedListItem
{
	template<typename U>
	friend class LinkedList;

public:
	LinkedListItem(T i) : m_item(i), m_next(NULL) {}
	~LinkedListItem()
	{
#ifdef CONSOLE
		if (m_item)
		{
			CONSOLE.println("~LinkedListItem - m_item is not NULL");
		}
		if (m_next)
		{
			CONSOLE.println("~LinkedListItem = m_next is not NULL");
		}
#endif
	}
	inline T getItem() { return m_item; }
	inline LinkedListItem<T>* getNext() { return m_next; }

private:
	T m_item;
	LinkedListItem<T>* m_next;
};

template<typename T>
class LinkedList
{
public:
	LinkedList() :
	m_first(NULL)
	{
	}
	~LinkedList() 
	{
		while (m_first)
		{
#ifdef CONSOLE
			CONSOLE.print("~LinkedList - items not cleaned up");
			if (m_first->m_item)
			{
				CONSOLE.print(" and item content is not NULL.");
			}
			CONSOLE.println();
#endif
			remove(m_first->m_item);
		}
	}

	void add(T item)
	{
		LinkedListItem<T>* container = new LinkedListItem<T>(item);
		if (m_first == NULL)
		{
			m_first = container;
		}
		else
		{
			for (LinkedListItem<T>* i = m_first; i; i = i->m_next)
			{
				if (!i->m_next)
				{
					i->m_next = container;
					break;
				}
			}
		}
	}

	T remove(T item)
	{
		LinkedListItem<T>* prev = NULL;
		for (LinkedListItem<T>* i = m_first; i; prev = i, i = i->m_next)
		{
			if (i->m_item == item)
			{
				if (i == m_first)
				{
					m_first = i->m_next;
				}
				else if (prev)
				{
					prev->m_next = i->m_next;
				}
				else
				{
#if defined CONSOLE
					CONSOLE.println("LinkedList: unexpected case");
#endif
				}
				i->m_item = NULL;
				i->m_next = NULL;
				delete i;
				return item;
			}
		}
#if defined CONSOLE
		CONSOLE.println("LinkedList: failed to remove item!");
#endif
		return NULL;
	}

	inline LinkedListItem<T>* getFirst() { return m_first; }

private:
	LinkedListItem<T>* m_first;
};

#endif
