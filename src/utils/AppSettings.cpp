/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "AppSettings.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>

void AppSettings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
    keyChanged(group() + key, value);
}

void AppSettings::remove(const QString &key)
{
    QSettings::remove(key);
    keyChanged(group() + key, QVariant());
}

bool AppSettings::addTrackingProperty(const QString &key, QObject *object, const char *propname)
{
    const QMetaObject *metaObject = object->metaObject();

    /* Get the meta-property */
    QMetaProperty property;
    if (propname)
        property = metaObject->property(metaObject->indexOfProperty(propname));
    else
        property = metaObject->userProperty();

    if (!property.isValid())
    {
        qWarning() << "AppSettings: Unknown property" << propname << "for object" << object->objectName()
                << "of class" << metaObject->className() << "with key" << key;
        return false;
    }

#ifdef QT_DEBUG
    /* Check for duplicates */
    QMultiHash<QObject*,QPair<QString,int> >::Iterator it = trackingObjectMap.find(object);
    for (; it != trackingObjectMap.end() && it.key() == object; ++it)
    {
        if (it->first == key && it->second == metaObject->indexOfProperty(propname))
        {
            qWarning() << "AppSettings: Duplicate tracking of key" << key << "by object" << object->objectName()
                    << "of class" << metaObject->className() << "for property" << property.name();
            break;
        }
    }
#endif

    /* Connect to the notify signal of that property */
    if (property.hasNotifySignal())
    {
        /* See the comment in the header for why this hack is necessary; what we're doing here is
         * determining how many properties are connected from this object, and choosing one of the
         * five available slots based on that. Because the order in QMultiHash is based on insertion
         * order, we can map this back in the slot. */
        char slot[] = { "1objectChanged?()" };
        int propsForObject = trackingObjectMap.count(object);
        if (propsForObject >= 5)
        {
            qWarning() << "AppSettings: Too many properties tracked from object" << object->objectName()
                    << "of class" << metaObject->className();
            return false;
        }
        /* Cheap conversion from a single-digit number to ASCII */
        slot[14] = (char)propsForObject + 48;

        QByteArray notify(property.notifySignal().signature());
        notify.prepend('2');
        bool ok = connect(object, notify.constData(), this, slot);
        if (!ok)
        {
            qWarning() << "AppSettings: Signal connection failed for property" << property.name() << "of object"
                    << object->objectName() << "of class" << metaObject->className() << "with key" << key;
            return false;
        }
    }

    if (!trackingObjectMap.contains(object))
        connect(object, SIGNAL(destroyed(QObject*)), SLOT(removeTrackingProperty(QObject*)));

    /* Insert key/object mappings */
    trackingKeyMap.insertMulti(key, object);
    trackingObjectMap.insertMulti(object, qMakePair(key, metaObject->indexOfProperty(propname)));

    return true;
}

void AppSettings::removeTrackingProperty(QObject *object)
{
    QMultiHash<QObject*,QPair<QString,int> >::Iterator it = trackingObjectMap.find(object);
    if (it == trackingObjectMap.end())
        return;

    /* Remove each tracker registered to this object from trackingKeyMap, and each entry
     * in trackingObjectMap */
    while (it != trackingObjectMap.end() && it.key() == object)
    {
        QMultiHash<QString,QObject*>::Iterator keyit = trackingKeyMap.find(it->first);
        for (; keyit != trackingKeyMap.end() && keyit.key() == it->first; ++keyit)
        {
            if (*keyit == object)
            {
                trackingKeyMap.erase(keyit);
                break;
            }
        }

        it = trackingObjectMap.erase(it);
    }

    /* Disconnect from all signals on the object */
    disconnect(object, 0, this, 0);
}

void AppSettings::keyChanged(const QString &key, const QVariant &value)
{
    QMultiHash<QString,QObject*>::Iterator it = trackingKeyMap.find(key);

    for (; it != trackingKeyMap.end() && it.key() == key; ++it)
    {
        if (*it == keyChangeSource)
            continue;

        QMultiHash<QObject*,QPair<QString,int> >::Iterator objit = trackingObjectMap.find(*it);
        while (objit != trackingObjectMap.end() && objit->first != key)
            ++objit;
        int propid = (objit == trackingObjectMap.end()) ? -1 : objit->second;
        if (propid < 0)
            continue;

        QMetaProperty property = (*it)->metaObject()->property(propid);
        if (!property.isValid())
            continue;

        /* Temporarily clear the property ID from this trackingObjectMap entry, used to ignore
         * the notification signal that may result from this property write */
        objit->second = -1;
        bool ok = property.write(*it, value);
        objit->second = propid;

        if (!ok)
        {
            qWarning() << "AppSettings: Property write failed for object" << (*it)->objectName()
                    << "of class" << (*it)->metaObject()->className() << "with key" << key;
        }
    }
}

void AppSettings::objectChanged(QObject *object, int index)
{
    QMultiHash<QObject*,QPair<QString,int> >::Iterator it = trackingObjectMap.find(object), itstart = it;
    if (it == trackingObjectMap.end())
        return;

    /* Index is the index of this property among the properties connected from this object.
     * In the QMultiHash, these are stored from the most-recently inserted to the least recently
     * inserted, so we need to invert the number and get the entry at that offset. */
    int propCount = 0;
    for (; it != trackingObjectMap.end() && it.key() == object; ++it, ++propCount);

    /* First connection (index=0) is the last in the map (propCount-1) */
    index = propCount - 1 - index;
    it = itstart + index;

    if (it->second < 0)
        return;

    QMetaProperty property = object->metaObject()->property(it->second);
    if (!property.isValid())
        return;

    QObject *old = keyChangeSource;
    keyChangeSource = object;
    setValue(it->first, property.read(object));
    keyChangeSource = old;
}
