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
	for(int n=0; n < mResolvers.count(); n++)
	{
		OpenNICResolver* resolver = mResolvers[n];
		resolver->decRef();
	}
	mResolvers.clear();
}

OpenNICResolverPool& OpenNICResolverPool::copy(OpenNICResolverPool& other)
{
	if ( &other != this )
	{
		for(int n=0; n < other.mResolvers.count(); n++)
		{
			append(other.mResolvers[n]);
		}
	}
	return *this;
}

/**
  * @brief export to a tring list format
  */
QStringList& OpenNICResolverPool::toStringList()
{
	mStringList.clear();
	for(int n=0; n < mResolvers.count(); n++)
	{
		OpenNICResolver* resolver = mResolvers[n];
		QString itemString = resolver->toString();
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
OpenNICResolverPool& OpenNICResolverPool::fromStringList(const QStringList items, QString kind)
{
	for(int n=0; n < items.count(); n++)
	{
		if (!items[n].trimmed().isEmpty())
		{
			QHostAddress host(items[n].trimmed());
			insort(host,kind);
		}
	}
	return *this;
}

/**
  * @brief from a IP list
  */
OpenNICResolverPool& OpenNICResolverPool::fromIPList(const QStringList items, QString kind)
{
	for(int n=0; n < items.count(); n++)
	{
		if (!items[n].trimmed().isEmpty())
		{
			OpenNICResolver* resolver = new OpenNICResolver(QHostAddress(items[n].trimmed()),kind);
			insort(resolver);
		}
	}
	return *this;
}

/**
  * @brief adjust the maximum history depth of all of the resolvers in the pool.
  */
void OpenNICResolverPool::setMaxHistoryDepth(int maxHistoryDepth)
{
	for(int n=0; n < mResolvers.count(); n++)
	{
		mResolvers[n]->setMaxQueryDepth(maxHistoryDepth);
	}
}

/**
  * @brief clear the pool
  */
void OpenNICResolverPool::clear()
{
	mResolvers.clear();
}

/**
  * @brief append an item
  */
bool OpenNICResolverPool::append(OpenNICResolver* resolver)
{
	resolver->incRef();
	if ( !contains(resolver) )
	{
		mResolvers.append(resolver);
		return true;
	}
	resolver->decRef();
	return false;
}

/**
  * @brief append an item
  */
bool OpenNICResolverPool::append(QHostAddress& host, QString kind)
{
	return append(new OpenNICResolver(host,kind));
}

/**
  * @brief append and sort
  */
bool OpenNICResolverPool::insort(OpenNICResolver* resolver)
{
	if ( append(resolver) )
	{
		sort();
		return true;
	}
	return false;
}

/**
  * @brief append and sort
  */
bool OpenNICResolverPool::insort(QHostAddress& host, QString kind)
{
	if ( append(host,kind) )
	{
		sort();
		return true;
	}
	return false;
}

/**
  * @brief swap the positions of one item with another
  */
void OpenNICResolverPool::swap(int a,int b)
{
	OpenNICResolver* t = mResolvers[a];
	mResolvers.replace(a,mResolvers[b]);
	mResolvers.replace(b,t);
}

/**
  * @brief score the items in the pool
  */
void OpenNICResolverPool::score()
{
	int nResolvers = mResolvers.count();
	double totalAverageLatency=0.0;
	double averageLatency=0.0;
	for(int n=0; n < nResolvers; n++)
	{
		OpenNICResolver* resolver = mResolvers[n];
		totalAverageLatency += resolver->averageLatency();
	}
	averageLatency = totalAverageLatency/(double)nResolvers;
	/** apply the scores... */
	for(int n=0; n < nResolvers; n++)
	{
		OpenNICResolver* resolver = mResolvers[n];
		double score = averageLatency/resolver->averageLatency();
		if (resolver->kind()=="T2") score = score * 2;
		resolver->setScore(score);
	}
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
				if ( mResolvers[(a=n)] < mResolvers[(b=(n+1))] )
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
bool OpenNICResolverPool::contains(OpenNICResolver* resolver)
{
	return contains(resolver->hostAddress());
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
int OpenNICResolverPool::indexOf(OpenNICResolver* resolver)
{
	return indexOf(resolver->hostAddress());
}

/**
  * @return the index of the item or -1
  */
int OpenNICResolverPool::indexOf(QHostAddress hostAddress)
{
	for(int n=0; n < mResolvers.count(); n++)
	{
		OpenNICResolver* other = mResolvers[n];
		if ( hostAddress == other->hostAddress() )
		{
			return n;
		}
	}
	return -1;
}



