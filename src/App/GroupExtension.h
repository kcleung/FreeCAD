/***************************************************************************
 *   Copyright (c) 2006 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#ifndef APP_GROUPEXTENSION_H
#define APP_GROUPEXTENSION_H

#include "FeaturePython.h"
#include "DocumentObject.h"
#include "PropertyLinks.h"
#include "DocumentObjectExtension.h"
#include <vector>

namespace App
{
class DocumentObjectGroup;
class GroupExtensionPy;

class AppExport GroupExtension : public DocumentObjectExtension
{
    EXTENSION_PROPERTY_HEADER(App::GroupExtension);

public:
    /// Constructor
    GroupExtension(void);
    virtual ~GroupExtension();

    /** @name Object handling  */
    //@{
    /** Adds an object of \a sType with \a pObjectName to the document this group belongs to and
     * append it to this group as well.
     */
    virtual DocumentObject *newObject(const char* sType, const char* pObjectName);

    /** Adds the object \a obj to this group. If the object is already in another group, it is withdrawn first.
     */
    virtual std::vector<DocumentObject*> addObject(DocumentObject* obj);
    /* Adds the objects \a objs to this group. Returns all objects that have been added.
     */
    virtual std::vector<DocumentObject*> addObjects(std::vector<DocumentObject*> obj);
   /**
     * @brief adoptObject: adds object to this group if it isn't in any other
     * group. Otherwise does nothing. This is useful for adding lists of
     * objects with internal container relationships.
     * @return returns true if object was adopted, false otherwise. (if re-adopting an own child, returns true too).
     */
    virtual bool adoptObject(DocumentObject* obj);

    /*override this function if you want only special objects
     */
    /**
     * @brief canAccept(object): tests if the given object can be accepted by this group.
     */
    virtual bool canAccept(DocumentObject* obj);
    /**
     * @brief allowObject(object): (deprecated) the call is redirected to GroupExtension::canAccept(DocumentObject* obj).
     */
    virtual bool allowObject(DocumentObject* obj);
    /**
     * @brief canCreate(type, pytype): tests if an object of the given type can be created in this group.
     * @param type: c++ type name, e.g. "Part::Primitive"
     * @param pytype: python type (free-form)
     * @return
     */
    virtual bool canCreate(const char* type, const char* pytype = "");
    /**
     * @brief allowObject(type, pytype): (deprecated) the call is redirected to GroupExtension::canCreate(const char* type, const char* pytype = "").
     * @param type: c++ type name, e.g. "Part::Primitive"
     * @param pytype: python type (free-form)
     * @return
     */
    virtual bool allowObject(const char* type, const char* pytype = "");
    
    /** Removes an object from this group. Returns all objects that have been removed.
     */
    virtual std::vector<DocumentObject*> removeObject(DocumentObject* obj);
    /** Removes objects from this group. Returns all objects that have been removed.
     */
    virtual std::vector<DocumentObject*> removeObjects(std::vector<DocumentObject*> obj);
    /** Removes all children objects from this group and the document.
     */
    virtual void removeObjectsFromDocument();
    /** Returns the object of this group with \a Name. If the group doesn't have such an object 0 is returned.
     * @note This method might return 0 even if the document this group belongs to contains an object with this name.
     */
    DocumentObject *getObject(const char* Name) const;
    /**
     * Checks whether the object \a obj is part of this group.
     * @param obj        the object to check for.
     * @param recursive  if true check also if the obj is child of some sub group (default is false).
     */
    bool hasObject(const DocumentObject* obj, bool recursive=false) const;
    /**
     * Checks whether this group object is a child (or sub-child)
     * of the given group object.
     */
    bool isChildOf(const GroupExtension*) const;
    /** Returns a list of all objects this group does have.
     */
    virtual std::vector<DocumentObject*> getObjects() const;
    virtual std::vector<DocumentObject*> getStaticObjects() const;
    virtual std::vector<DocumentObject*> getDynamicObjects() const;

    /** Returns a list of all objects of \a typeId this group does have.
     */
    std::vector<DocumentObject*> getObjectsOfType(const Base::Type& typeId) const;
    /** Returns the number of objects of \a typeId this group does have.
     */
    int countObjectsOfType(const Base::Type& typeId) const;
    /** Returns the object group of the document which the given object \a obj is part of.
     * In case this object is not part of a group 0 is returned. 
     * @note This only returns objects that are normal groups, not any special derived type 
     * like GeoFeatureGroups or OriginGroups. To retrieve those please use their appropriate functions
     */
    static DocumentObject* getGroupOfObject(const DocumentObject* obj);
    //@}
    
    virtual PyObject* getExtensionPyObject(void);

    virtual void extensionOnChanged(const Property* p) override;
    
    /// Properties
    PropertyLinkList Group;

private:
    void removeObjectFromDocument(DocumentObject*);
};


template<typename ExtensionT>
class GroupExtensionPythonT : public ExtensionT {
         
public:
    
    GroupExtensionPythonT() {}
    virtual ~GroupExtensionPythonT() {}

    //override the documentobjectextension functions to make them available in python 
    virtual bool canAccept(DocumentObject* obj) override {
        Py::Object pyobj = Py::asObject(obj->getPyObject());
        EXTENSION_PROXY_ONEARG(canAccept, pyobj);
                
        if(result.isNone())
            return ExtensionT::canAccept(obj);
        
        if(result.isBoolean())
            return result.isTrue();
        
        return false;
    };


    virtual bool allowObject(DocumentObject* obj) override {
        return canAccept(obj);
    };

    virtual bool canCreate(const char* type, const char* pytype) override {
        Py::String arg0(type);
        Py::String arg1(pytype ? pytype : "");
        EXTENSION_PROXY_FIRST(canCreate)
        Py::Tuple args(2);
        args.setItem(0, arg0);
        args.setItem(0, arg1);
        EXTENSION_PROXY_SECOND(canCreate)
        Py::Tuple args(3);
        args.setItem(0, Py::Object(this->getExtensionPyObject(), true));
        args.setItem(1, arg0);
        args.setItem(2, arg1);
        EXTENSION_PROXY_THIRD()

        if(result.isNone())
            return ExtensionT::canCreate(type, pytype);

        if(result.isBoolean())
            return result.isTrue();

        return false;
    };


    virtual bool allowObject(const char* type, const char* pytype) override {
        return canCreate(type, pytype);
    };

};

typedef ExtensionPythonT<GroupExtensionPythonT<GroupExtension>> GroupExtensionPython;

} //namespace App


#endif // APP_GROUPEXTENSION_H
