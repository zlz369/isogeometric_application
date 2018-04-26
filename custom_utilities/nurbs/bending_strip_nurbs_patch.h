//
//   Project Name:        Kratos
//   Last Modified by:    $Author: hbui $
//   Date:                $Date: 22 Apr 2018 $
//   Revision:            $Revision: 1.0 $
//
//

#if !defined(KRATOS_ISOGEOMETRIC_APPLICATION_BENDING_STRIP_NURBS_PATCH_H_INCLUDED )
#define  KRATOS_ISOGEOMETRIC_APPLICATION_BENDING_STRIP_NURBS_PATCH_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_utilities/bending_strip_patch.h"
#include "custom_utilities/multipatch.h"
#include "custom_utilities/nurbs/knot_array_1d.h"
#include "custom_utilities/nurbs/bsplines_fespace.h"
#include "custom_utilities/nurbs/bsplines_fespace_library.h"
#include "custom_utilities/nurbs/structured_control_grid.h"


namespace Kratos
{

/**
This class represents an isogeometric bending strip patch connecting two NURBS patches.
REF: Kiendl et al, The bending strip method for isogeometric analysis of Kirchhoff–Love shell structures comprised of multiple patches.
 */
template<int TDim>
class BendingStripNURBSPatch : public BendingStripPatch<TDim>
{
public:
    /// Pointer definition
    KRATOS_CLASS_POINTER_DEFINITION(BendingStripNURBSPatch);

    typedef Patch<TDim> PatchType;
    typedef KnotArray1D<double> knot_container_t;
    typedef BendingStripPatch<TDim> BaseType;
    typedef typename BaseType::ControlPointType ControlPointType;

    /// Default Constructor
    BendingStripNURBSPatch(const std::size_t& Id, const int& Order) : BaseType(Id, Order)
    {
    }

    /// Full Constructor
    /// Note that the two patches must represent the NURBS patches
    /// Right now this constructor only support even order bending strip
    /// In this constructor the knot vector on the boundary will be taken as the same as the original patch
    BendingStripNURBSPatch(const std::size_t& Id,
        typename PatchType::Pointer pPatch1, const BoundarySide& side1,
        typename PatchType::Pointer pPatch2, const BoundarySide& side2,
        const int& Order) : BaseType(Id, pPatch1, side1, pPatch2, side2, Order)
    {
        // check if the order is even
        if (this->NormalOrder()%2 != 0)
            KRATOS_THROW_ERROR(std::logic_error, "The strip order is not even, but", this->NormalOrder())

        // get the boundary patches
        typename Patch<TDim-1>::Pointer pBPatch1 = pPatch1->ConstructBoundaryPatch(side1);
        typename Patch<TDim-1>::Pointer pBPatch2 = pPatch2->ConstructBoundaryPatch(side2);

        // check if two patches are the same
        if (!(pBPatch1->IsSame(*pBPatch2)))
        {
            KRATOS_WATCH(*pBPatch1)
            KRATOS_WATCH(*pBPatch2)
            KRATOS_THROW_ERROR(std::logic_error, "The two boundary patches are not the same", "")
        }

        // get the FESpace of boundary patch 1
        typename BSplinesFESpace<TDim-1>::Pointer pBFESpace = boost::dynamic_pointer_cast<BSplinesFESpace<TDim-1> >(pBPatch1->pFESpace());

        // construct the FESpace
        typename BSplinesFESpace<TDim>::Pointer pFESpace = typename BSplinesFESpace<TDim>::Pointer(new BSplinesFESpace<TDim>());

        for (std::size_t dim = 0; dim < TDim-1; ++dim)
        {
            knot_container_t knot_vector = pBFESpace->KnotVector(dim);
            pFESpace->SetKnotVector(dim, knot_vector);
            pFESpace->SetInfo(dim, pBFESpace->Number(dim), pBFESpace->Order(dim));
        }

        knot_container_t w_knots = BSplinesFESpaceLibrary::CreatePrimitiveOpenKnotVector(this->NormalOrder());
        pFESpace->SetKnotVector(TDim-1, w_knots);
        pFESpace->SetInfo(TDim-1, this->NormalOrder()+1, this->NormalOrder());

        pFESpace->ResetFunctionIndices();

        /*****Set the FESpace*****/
        this->SetFESpace(pFESpace);
        /*************************/

        typename StructuredControlGrid<TDim-1, ControlPointType>::Pointer pBControlPointGrid = boost::dynamic_pointer_cast<StructuredControlGrid<TDim-1, ControlPointType> >( pBPatch1->ControlPointGridFunction().pControlGrid() );
        std::vector<std::size_t> strip_sizes(TDim);
        for (std::size_t dim = 0; dim < TDim-1; ++dim)
            strip_sizes[dim] = pBControlPointGrid->Size(dim);
        strip_sizes[TDim-1] = this->NormalOrder()+1;

        // construct the control point grid and assign the respective grid function
        std::vector<typename StructuredControlGrid<TDim-1, ControlPointType>::Pointer> pSlicedControlPointGrids;

        pSlicedControlPointGrids = this->ExtractSlicedControlGrids<ControlPointType>( pPatch1->ControlPointGridFunction().pControlGrid(), pPatch2->ControlPointGridFunction().pControlGrid(), pBPatch1->ControlPointGridFunction().pControlGrid() );

        typename StructuredControlGrid<TDim, ControlPointType>::Pointer pStripControlPointGrid = typename StructuredControlGrid<TDim, ControlPointType>::Pointer( new StructuredControlGrid<TDim, ControlPointType>(strip_sizes) );

        pStripControlPointGrid->CopyFrom(TDim-1, pSlicedControlPointGrids);

        this->CreateControlPointGridFunction(pStripControlPointGrid);

        //// TODO: transfer other control values

        /**************set the indices from the parent**********************/
        std::vector<std::size_t> parent_indices = GetIndicesFromParent();
        pFESpace->ResetFunctionIndices(parent_indices);
    }

    /// Full Constructor
    /// Note that the two patches must represent the NURBS patches
    /// Right now this constructor only support even order bending strip
    /// In this constructor the knot vector on the strip patch can be specified
    BendingStripNURBSPatch(const std::size_t& Id,
        typename PatchType::Pointer pPatch1, const BoundarySide& side1,
        typename PatchType::Pointer pPatch2, const BoundarySide& side2,
        const std::vector<int>& Orders) : BaseType(Id, pPatch1, side1, pPatch2, side2, Orders[TDim-1])
    {
        // check if the order is even
        if (this->NormalOrder()%2 != 0)
            KRATOS_THROW_ERROR(std::logic_error, "The strip order is not even, but", this->NormalOrder())

        // get the boundary patches
        typename Patch<TDim-1>::Pointer pBPatch1 = pPatch1->ConstructBoundaryPatch(side1);
        typename Patch<TDim-1>::Pointer pBPatch2 = pPatch2->ConstructBoundaryPatch(side2);

        // check if two patches are the same
        if (!(pBPatch1->IsSame(*pBPatch2)))
        {
            KRATOS_WATCH(*pBPatch1)
            KRATOS_WATCH(*pBPatch2)
            KRATOS_THROW_ERROR(std::logic_error, "The two boundary patches are not the same", "")
        }

        // get the FESpace of boundary patch 1
        typename BSplinesFESpace<TDim-1>::Pointer pBFESpace = boost::dynamic_pointer_cast<BSplinesFESpace<TDim-1> >(pBPatch1->pFESpace());

        // construct the FESpace
        typename BSplinesFESpace<TDim>::Pointer pFESpace = typename BSplinesFESpace<TDim>::Pointer(new BSplinesFESpace<TDim>());

        for (std::size_t dim = 0; dim < TDim-1; ++dim)
        {
            knot_container_t knot_vector = BSplinesFESpaceLibrary::CreateUniformOpenKnotVector(pBFESpace->Number(dim), Orders[dim]);
            pFESpace->SetKnotVector(dim, knot_vector);
            pFESpace->SetInfo(dim, pBFESpace->Number(dim), Orders[dim]);
        }

        knot_container_t w_knots = BSplinesFESpaceLibrary::CreatePrimitiveOpenKnotVector(this->NormalOrder());
        pFESpace->SetKnotVector(TDim-1, w_knots);
        pFESpace->SetInfo(TDim-1, this->NormalOrder()+1, this->NormalOrder());

        pFESpace->ResetFunctionIndices();

        /*****Set the FESpace*****/
        this->SetFESpace(pFESpace);
        /*************************/

        typename StructuredControlGrid<TDim-1, ControlPointType>::Pointer pBControlPointGrid = boost::dynamic_pointer_cast<StructuredControlGrid<TDim-1, ControlPointType> >( pBPatch1->ControlPointGridFunction().pControlGrid() );
        std::vector<std::size_t> strip_sizes(TDim);
        for (std::size_t dim = 0; dim < TDim-1; ++dim)
            strip_sizes[dim] = pBControlPointGrid->Size(dim);
        strip_sizes[TDim-1] = this->NormalOrder()+1;

        // construct the control point grid and assign the respective grid function
        std::vector<typename StructuredControlGrid<TDim-1, ControlPointType>::Pointer> pSlicedControlPointGrids;

        pSlicedControlPointGrids = this->ExtractSlicedControlGrids<ControlPointType>( pPatch1->ControlPointGridFunction().pControlGrid(), pPatch2->ControlPointGridFunction().pControlGrid(), pBPatch1->ControlPointGridFunction().pControlGrid() );

        typename StructuredControlGrid<TDim, ControlPointType>::Pointer pStripControlPointGrid = typename StructuredControlGrid<TDim, ControlPointType>::Pointer( new StructuredControlGrid<TDim, ControlPointType>(strip_sizes) );

        pStripControlPointGrid->CopyFrom(TDim-1, pSlicedControlPointGrids);

        this->CreateControlPointGridFunction(pStripControlPointGrid);

        //// TODO: transfer other control values

        /**************set the indices from the parent**********************/
        std::vector<std::size_t> parent_indices = GetIndicesFromParent();
        pFESpace->ResetFunctionIndices(parent_indices);
    }

    /// Destructor
    virtual ~BendingStripNURBSPatch()
    {
        #ifdef DEBUG_DESTROY
        std::cout << Type() << ", Id = " << Id() << ", Addr = " << this << " is destroyed" << std::endl;
        #endif
    }

    /// Get the string representing the type of the patch
    virtual std::string Type() const
    {
        return StaticType();
    }

    /// Get the string representing the type of the patch
    static std::string StaticType()
    {
        std::stringstream ss;
        ss << "BendingStripNURBSPatch" << TDim << "D";
        return ss.str();
    }

    /// Get the indices of the control points on this strip from the parent patch
    virtual std::vector<std::size_t> GetIndicesFromParent() const
    {
        std::vector<std::size_t> func_indices;

        for (std::size_t i = 0; i < this->NormalOrder()/2; ++i)
        {
            std::vector<std::size_t> indices = this->pPatch1()->pFESpace()->ExtractBoundaryFunctionIndices(this->Side1(), i+1);
            func_indices.insert(func_indices.end(), indices.begin(), indices.end());
        }

        std::vector<std::size_t> indices = this->pPatch1()->pFESpace()->ExtractBoundaryFunctionIndices(this->Side1(), 0);
        func_indices.insert(func_indices.end(), indices.begin(), indices.end());

        for (std::size_t i = 0; i < this->NormalOrder()/2; ++i)
        {
            std::vector<std::size_t> indices = this->pPatch2()->pFESpace()->ExtractBoundaryFunctionIndices(this->Side2(), i+1);
            func_indices.insert(func_indices.end(), indices.begin(), indices.end());
        }

        return func_indices;        
    }

    template<typename TDataType>
    std::vector<typename StructuredControlGrid<TDim-1, TDataType>::Pointer> ExtractSlicedControlGrids(
        typename ControlGrid<TDataType>::Pointer pControlGrid1,
        typename ControlGrid<TDataType>::Pointer pControlGrid2,
        typename ControlGrid<TDataType>::Pointer pBControlGrid
    ) const
    {
        std::vector<typename StructuredControlGrid<TDim-1, TDataType>::Pointer> pSlicedControlGrids;

        typename StructuredControlGrid<TDim, TDataType>::Pointer psControlGrid1 = boost::dynamic_pointer_cast<StructuredControlGrid<TDim, TDataType> >( pControlGrid1 );
        typename StructuredControlGrid<TDim, TDataType>::Pointer psControlGrid2 = boost::dynamic_pointer_cast<StructuredControlGrid<TDim, TDataType> >( pControlGrid2 );
        typename StructuredControlGrid<TDim-1, TDataType>::Pointer psBControlGrid = boost::dynamic_pointer_cast<StructuredControlGrid<TDim-1, TDataType> >( pBControlGrid );

        for (std::size_t i = 0; i < this->NormalOrder()/2; ++i)
            pSlicedControlGrids.push_back(psControlGrid1->Get(this->Side1(), i+1));

        pSlicedControlGrids.push_back(psBControlGrid);

        for (std::size_t i = 0; i < this->NormalOrder()/2; ++i)
            pSlicedControlGrids.push_back(psControlGrid2->Get(this->Side2(), i+1));

        return pSlicedControlGrids;
    }
};

template<>
class BendingStripNURBSPatch<1> : public BendingStripPatch<1>
{
public:
    /// Pointer definition
    KRATOS_CLASS_POINTER_DEFINITION(BendingStripNURBSPatch);

    typedef Patch<1> PatchType;
    typedef BendingStripPatch<1> BaseType;

    BendingStripNURBSPatch(const std::size_t& Id, const int& Order) : BaseType(Id, Order)
    {
    }

    BendingStripNURBSPatch(const std::size_t& Id,
        typename PatchType::Pointer pPatch1, const BoundarySide& side1,
        typename PatchType::Pointer pPatch2, const BoundarySide& side2,
        const int& Order) : BaseType(Id, pPatch1, side1, pPatch2, side2, Order)
    {
        // TODO
    }
};

/// output stream function
template<int TDim>
inline std::ostream& operator <<(std::ostream& rOStream, const BendingStripNURBSPatch<TDim>& rThis)
{
    rOStream << "-------------Begin BendingStripNURBSPatchInfo-------------" << std::endl;
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);
    rOStream << std::endl;
    rOStream << "-------------End BendingStripNURBSPatchInfo-------------";
    return rOStream;
}

} // namespace Kratos.

#undef DEBUG_DESTROY

#endif // KRATOS_ISOGEOMETRIC_APPLICATION_BENDING_STRIP_NURBS_PATCH_H_INCLUDED defined

