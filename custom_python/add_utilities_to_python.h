/*
LICENSE: see isogeometric_application/LICENSE.txt
*/

//
//   Project Name:        Kratos
//   Last Modified by:    $Author: hbui $
//   Date:                $Date: Aug 18, 2013 $
//   Revision:            $Revision: 1.1 $
//
//


#if !defined(KRATOS_ISOGEOMETRIC_APPLICATION_ADD_CUSTOM_UTILITIES_TO_PYTHON_H_INCLUDED )
#define  KRATOS_ISOGEOMETRIC_APPLICATION_ADD_CUSTOM_UTILITIES_TO_PYTHON_H_INCLUDED



// System includes


// External includes


// Project includes
#include "includes/define.h"


namespace Kratos
{

namespace Python
{

template<class TPatchType>
TPatchType& GetReference(typename TPatchType::Pointer& dummy)
{
    return *dummy;
}

void  IsogeometricApplication_AddCustomUtilitiesToPython();
void  IsogeometricApplication_AddNURBSToPython();
void  IsogeometricApplication_AddHBSplinesToPython();
void  IsogeometricApplication_AddIsogeometricStructuresToPython();

}  // namespace Python.

}  // namespace Kratos.

#endif // KRATOS_ISOGEOMETRIC_APPLICATION_ADD_CUSTOM_UTILITIES_TO_PYTHON_H_INCLUDED  defined

