/*
 * This file is a part of OpenNIC Wizard
 * Copywrong (c) 2012-2022 Mike Sharkey
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 1776):
 * <mike@8bitgeek.net> wrote this file.
 * As long as you retain this notice you can do whatever you want with this
 * stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return. ~ Mike Sharkey
 * ----------------------------------------------------------------------------
 */
#include "opennicresolverpool.h"
#include "opennicsystem.h"
#include "opennicserver.h"

OpenNICResolverPool::OpenNICResolverPool(QObject *parent)
: QObject(parent)
{
	QObject::connect(&mScriptEngine,SIGNAL(signalHandlerException(QScriptValue)),this,SLOT(signalHandlerException(QScriptValue)));
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
QStringList OpenNICResolverPool::toStringList()
{
	QStringList rc;
	for(int n=0; n < mResolvers.count(); n++)
	{
		OpenNICResolver* resolver = mResolvers[n];
		QString itemString = resolver->toString();
		rc.append(itemString);
	}
	return rc;
}


/**
  * @brief export to a tring list format
  *
  */
QStringList OpenNICResolverPool::toStringList(QString select)
{
	QStringList rc;
	int idx = indexOf(QHostAddress(select));
	if ( idx >= 0 )
	{
		OpenNICResolver* resolver = at(idx);
		for(int n=0; n < resolver->queries().count(); n++)
		{
			rc << resolver->queries().at(n)->toString();
		}
	}
	return rc;
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
  * @brief calculate the average latency
  * @param min minimum average resolver latency
  * @param max maximum average resolver latency
  * @return average resolver latency
  */
double OpenNICResolverPool::latency(double& min, double& max)
{
	int nResolvers = mResolvers.count();
	min=0.0;
	max=0.0;
	if (nResolvers > 0 )
	{
		double total=0.0;
		double averageLatency=0.0;
		for(int n=0; n < nResolvers; n++)
		{
			OpenNICResolver* resolver = mResolvers[n];
			double resolverAverageLatency = resolver->averageLatency();
			total += resolverAverageLatency;
			if (n==0)
			{
				min = max = resolverAverageLatency;
			}
			else
			{
				if (resolverAverageLatency < min) min = resolverAverageLatency;
				if (resolverAverageLatency > max) max = resolverAverageLatency;
			}
		}
		averageLatency = total/(double)nResolvers;
		return averageLatency;
	}
	return 0.0;
}

/**
  * @brief script exception handler
  */
void OpenNICResolverPool::signalHandlerException(const QScriptValue& exception)
{
	OpenNICServer::log(exception.toString());
}


/**
  * @brief score the resolver using the internal rules
  * @return a score
  */
double OpenNICResolverPool::scoreResolverInternal(OpenNICResolver* resolver, double /* averagePoolLatency */, double /* minPoolLatency */, double maxPoolLatency )
{
	/* perform the scoring */
	double resolverAverageLatency = resolver->averageLatency();
	if (resolverAverageLatency < 0.0)
	{
		resolverAverageLatency = maxPoolLatency;
	}
	double score = (maxPoolLatency - resolverAverageLatency);
	if (resolver->kind()=="T1")							score /= 1.5;
	if (resolver->kind()=="T2")							score *= 1.5;
	if (resolver->resolvesNIC("opennic"))				score *= 1.75;
	if (resolver->status() == OpenNICResolver::Red)		score /= 2;
	if (resolver->status() == OpenNICResolver::Yellow)	score /= 1.25;
	/* return the result */
	return score;
}

/**
  * @brief score the resolver using the internal rules
  * @return a score
  */
double OpenNICResolverPool::scoreResolverScript(OpenNICResolver* resolver, double averagePoolLatency, double minPoolLatency, double maxPoolLatency )
{
	mScriptEngine.globalObject().setProperty("averagePoolLatency",averagePoolLatency);
	mScriptEngine.globalObject().setProperty("minPoolLatency",minPoolLatency);
	mScriptEngine.globalObject().setProperty("maxPoolLatency",maxPoolLatency);
	mScriptEngine.globalObject().setProperty("resolverAverageLatency",resolver->averageLatency());
	mScriptEngine.globalObject().setProperty("resolverKind",resolver->kind());
	mScriptEngine.globalObject().setProperty("resolverStatus",resolver->status());
	QScriptValue result = mScriptEngine.evaluate(OpenNICServer::scoreRules());
	return result.toNumber();
}


/**
  * @brief score the items in the pool
  */
void OpenNICResolverPool::score()
{
	int nResolvers = mResolvers.count();
	if (nResolvers > 0 )
	{
		OpenNICResolver* resolver;
		double resolverAverageLatency;
		double score;
		double min,max;
		double averagePoolLatency = latency(min,max);
		OpenNICServer::log("min="+QString::number(min)+" max="+QString::number(max));
		/** apply the scores... */
		for(int n=0; n < nResolvers; n++)
		{
			resolver = mResolvers[n];
			resolverAverageLatency = resolver->averageLatency();
			if (resolverAverageLatency < 0.0)
			{
				resolverAverageLatency = max;
			}
			if (OpenNICServer::scoreInternal())
			{
				score = scoreResolverInternal(resolver,averagePoolLatency,min,max);
			}
			else
			{
				score = scoreResolverScript(resolver,averagePoolLatency,min,max);
			}
			/* store the result */
			resolver->setScore(score);
		}
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
                a = OpenNICSystem::instance()->random(0,count()-1);
                b = OpenNICSystem::instance()->random(0,count()-1);
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
		int deltaA;
		for(a=0; a < count()-1; a++)
		{
			deltaA=a;
			for(b=a+1; b < count(); b++)
			{
				if ( mResolvers[deltaA]->score() < mResolvers[b]->score() )
				{
					deltaA=b;
				}
			}
			swap(a,deltaA);
		}
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



