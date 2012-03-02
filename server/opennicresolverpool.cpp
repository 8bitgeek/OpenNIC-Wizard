/*
 * Copyright (c) 2012 Mike Sharkey <michael_sharkey@firstclass.com>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Mike Sharkey wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 */
#include "opennicresolverpool.h"
#include "opennicsystem.h"

OpenNICResolverPool::OpenNICResolverPool(QObject *parent)
: QObject(parent)
{
}

/**
  * @brief Clean up the pool.
  */
OpenNICResolverPool::~OpenNICResolverPool()
{
	mItems.clear();
}

OpenNICResolverPool& OpenNICResolverPool::copy(OpenNICResolverPool& other)
{
	mItems		= other.mItems;
	return *this;
}

/**
  * @brief export to a tring list format
  */
QStringList& OpenNICResolverPool::toStringList()
{
	mStringList.clear();
	for(int n=0; n < count(); n++)
	{
		OpenNICResolverPoolItem& item = mItems[n];
		QString itemString = item.toString();
		mStringList.append(itemString);
	}
	return mStringList;
}

OpenNICResolverPool& OpenNICResolverPool::operator<<(const QStringList& strings)
{
	return fromStringList(strings);
}


/**
  * @brief from a string list
  */
OpenNICResolverPool& OpenNICResolverPool::fromStringList(const QStringList items)
{
	for(int n=0; n < items.count(); n++)
	{
		if (!items[n].trimmed().isEmpty())
		{
			QHostAddress item(items[n].trimmed());
			insort(item);
		}
	}
	return *this;
}

/**
  * @brief adjust the maximum history depth of all of the resolvers in the pool.
  */
void OpenNICResolverPool::setMaxHistoryDepth(int maxHistoryDepth)
{
	for(int n=0; n < mItems.count(); n++)
	{
		mItems[n].setMaxHistoryDepth(maxHistoryDepth);
	}
}

/**
  * @brief clear the pool
  */
void OpenNICResolverPool::clear()
{
	mItems.clear();
}

/**
  * @brief set active state
  */
void OpenNICResolverPool::setActive(bool active)
{
	for(int n=0; n < count(); n++)
	{
		mItems[n].setActive(active);
	}
}

/**
  * @brief append an item
  */
void OpenNICResolverPool::append(OpenNICResolverPoolItem item)
{
	if ( !contains(item) )
	{
		mItems.append(item);
	}
}

/**
  * @brief append and sort
  */
void OpenNICResolverPool::insort(OpenNICResolverPoolItem item)
{
	if ( !contains(item) )
	{
		mItems.append(item);
		sort();
	}
}

/**
  * @brief swap the positions of one item with another
  */
void OpenNICResolverPool::swap(int a,int b)
{
	OpenNICResolverPoolItem t = mItems[a];
	mItems.replace(a,mItems[b]);
	mItems.replace(b,t);
}

/**
  * @brief opposite of sort()
  */
void OpenNICResolverPool::randomize()
{
	if ( count() >= 2 )
	{
		for(int n=0; n < count(); n++)
		{
			int a, b;
			// get un-equal a, b....
			do
			{
				a = OpenNICSystem::random(0,count()-1);
				b = OpenNICSystem::random(0,count()-1);
			} while (a==b);
			swap(a,b);
		}
	}
}

/**
  * @brief sort the pool
  */
void OpenNICResolverPool::sort()
{
	if (count() >= 2)
	{
		int a,b;
		bool sorted;
		int nCount = count()-1;
		do
		{
			sorted=true;
			for(int n=0; n < nCount; n++ )
			{
				if ( mItems[(a=n)] < mItems[(b=(n+1))] )
				{
					sorted=false;
					swap(a,b);
				}
			}
		} while (!sorted);
	}
}

/**
  * @brief determine if the pool contains this item (by IP address)
  */
bool OpenNICResolverPool::contains(OpenNICResolverPoolItem& item)
{
	return contains(item.hostAddress());
}

/**
  * @brief detemine if the pool contains this item (by IP address)
  */
bool OpenNICResolverPool::contains(QHostAddress item)
{
	return indexOf(item) >= 0;
}

/**
  * @return the index of the item or -1
  */
int OpenNICResolverPool::indexOf(OpenNICResolverPoolItem& item)
{
	return indexOf(item.hostAddress());
}

/**
  * @return the index of the item or -1
  */
int OpenNICResolverPool::indexOf(QHostAddress hostAddress)
{
	for(int n=0; n < mItems.count(); n++)
	{
		OpenNICResolverPoolItem& other = mItems[n];
		if ( hostAddress == other.hostAddress() )
		{
			return n;
		}
	}
	return -1;
}



