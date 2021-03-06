/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Vincent Chabannes <vincent.chabannes@feelpp.org>
       Date: 2014-06-04

  Copyright (C) 2014 Université Joseph Fourier (Grenoble I)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
   \file thermodyn.hpp
   \author Vincent Chabannes <vincent.chabannes@feelpp.org>
   \date 2014-06-04
 */


#ifndef FEELPP_THERMODYNAMICSBASE_HPP
#define FEELPP_THERMODYNAMICSBASE_HPP 1



#include <feel/feeldiscr/functionspace.hpp>
#include <feel/feelfilters/exporter.hpp>
//#include <feel/feelvf/vf.hpp>
#include <feel/feelts/bdf.hpp>

#include <feel/feelmodels/modelcore/modelnumerical.hpp>
#include <feel/feelmodels/modelcore/markermanagement.hpp>
#include <feel/feelmodels/modelcore/options.hpp>
#include <feel/feelmodels/modelalg/modelalgebraicfactory.hpp>



namespace Feel
{
namespace FeelModels
{


template< typename ConvexType, typename BasisTemperatureType>
class ThermoDynamicsBase : public ModelNumerical,
                           public MarkerManagementDirichletBC,
                           public MarkerManagementNeumannBC

    {
    public:
        typedef ModelNumerical super_type;
        typedef ThermoDynamicsBase<ConvexType,BasisTemperatureType> self_type;
        typedef boost::shared_ptr<self_type> self_ptrtype;
        //___________________________________________________________________________________//
        // mesh
        typedef ConvexType convex_type;
        static const uint16_type nDim = convex_type::nDim;
        static const uint16_type nOrderGeo = convex_type::nOrder;
        typedef Mesh<convex_type> mesh_type;
        typedef boost::shared_ptr<mesh_type> mesh_ptrtype;
        // basis
        static const uint16_type nOrderPoly = BasisTemperatureType::nOrder;
        typedef BasisTemperatureType basis_temperature_type;
        typedef Lagrange<nOrderPoly, Vectorial,Continuous,PointSetFekete> basis_velocityconvection_type;
        // function space temperature
        typedef FunctionSpace<mesh_type, bases<basis_temperature_type> > space_temperature_type;
        typedef boost::shared_ptr<space_temperature_type> space_temperature_ptrtype;
        typedef typename space_temperature_type::element_type element_temperature_type;
        typedef boost::shared_ptr<element_temperature_type> element_temperature_ptrtype;
        // function space velocity convection
        typedef FunctionSpace<mesh_type, bases<basis_velocityconvection_type> > space_velocityconvection_type;
        typedef boost::shared_ptr<space_velocityconvection_type> space_velocityconvection_ptrtype;
        typedef typename space_velocityconvection_type::element_type element_velocityconvection_type;
        typedef boost::shared_ptr<element_velocityconvection_type> element_velocityconvection_ptrtype;
        // time scheme
        typedef Bdf<space_temperature_type>  bdf_temperature_type;
        typedef boost::shared_ptr<bdf_temperature_type> bdf_temperature_ptrtype;
        // exporter
        typedef Exporter<mesh_type,nOrderGeo> export_type;
        typedef boost::shared_ptr<export_type> export_ptrtype;

        // algebraic solver
        typedef ModelAlgebraicFactory model_algebraic_factory_type;
        typedef boost::shared_ptr< model_algebraic_factory_type > model_algebraic_factory_ptrtype;


        ThermoDynamicsBase( std::string __prefix,
                            bool __buildMesh,
                            WorldComm const& __worldComm,
                            std::string __subPrefix,
                            std::string __appliShortRepository );

        std::string fileNameMeshPath() const { return prefixvm(this->prefix(),"ThermoDynamicsMesh.path"); }
        //___________________________________________________________________________________//
        // mesh, space, element temperature
        mesh_ptrtype const& mesh() const { return M_mesh; }
        space_temperature_ptrtype const& spaceTemperature() const { return M_Xh; }
        element_temperature_ptrtype const& fieldTemperature() const { return M_fieldTemperature; }
        element_velocityconvection_ptrtype const& fieldVelocityConvection() const { return M_fieldVelocityConvection; }
        bool fieldVelocityConvectionIsUsed() const { return M_fieldVelocityConvectionIsUsed; }
        bool fieldVelocityConvectionIsIncompressible() const { return M_fieldVelocityConvectionIsIncompressible; }
        void setFieldVelocityConvectionIsUsed(bool b) { M_fieldVelocityConvectionIsUsed=b; }
        bool fieldVelocityConvectionIsOperational() const { return (M_fieldVelocityConvection.use_count() > 0); }
        bool fieldVelocityConvectionIsUsedAndOperational() const { return this->fieldVelocityConvectionIsUsed() && this->fieldVelocityConvectionIsOperational(); }
        void setFieldVelocityConvectionIsIncompressible(bool b) { M_fieldVelocityConvectionIsIncompressible=b; }
        //___________________________________________________________________________________//
        // physical parameters
        double thermalConductivity() const { return M_thermalConductivity; }
        double rho() const { return M_rho; } // density
        double heatCapacity() const { return M_heatCapacity; }
        void setThermalConductivity( double d ) { M_thermalConductivity=d; }
        void setRho( double d ) { M_rho=d; }
        void setHeatCapacity( double d ) { M_heatCapacity=d; }
        //___________________________________________________________________________________//
        // algebraic data and solver
        backend_ptrtype const& backend() const { return  M_backend; }
        BlocksBaseVector<double> const& blockVectorSolution() const { return M_blockVectorSolution; }
        BlocksBaseVector<double> & blockVectorSolution() { return M_blockVectorSolution; }
        size_type nLocalDof() const;
        model_algebraic_factory_ptrtype const& algebraicFactory() const { return M_algebraicFactory; }
        model_algebraic_factory_ptrtype & algebraicFactory() { return M_algebraicFactory; }
        //___________________________________________________________________________________//
        // exporter
        void exportResults() { this->exportResults( this->currentTime() ); }
        void exportResults( double time );
        //___________________________________________________________________________________//
        // time step scheme
        bdf_temperature_ptrtype const& timeStepBdfTemperature() const { return M_bdfTemperature; }
        boost::shared_ptr<TSBase> timeStepBase() { return this->timeStepBdfTemperature(); }
        boost::shared_ptr<TSBase> timeStepBase() const { return this->timeStepBdfTemperature(); }
        void updateBdf();
        void updateTimeStep() { this->updateBdf(); }
        //___________________________________________________________________________________//

        boost::shared_ptr<std::ostringstream> getInfo() const;

        virtual void loadConfigBCFile() = 0;
        virtual void loadConfigMeshFile(std::string const& geofilename) = 0;

        void loadParameterFromOptionsVm();
        void createMesh();
        void createFunctionSpaces();
        void createTimeDiscretisation();
        void createExporters();
        BlocksBaseGraphCSR buildBlockMatrixGraph() const;
        int nBlockMatrixGraph() const { return 1; }
        void init( bool buildModelAlgebraicFactory, model_algebraic_factory_type::appli_ptrtype const& app );
        void updateForUseFunctionSpacesVelocityConvection();
        void restartExporters();

        void build();
        void loadMesh( mesh_ptrtype mesh );

        //___________________________________________________________________________________//
        //___________________________________________________________________________________//
        // apply assembly and solver
        virtual void solve();

        void updateLinearPDE( DataUpdateLinear & data ) const;
        virtual void updateWeakBCLinearPDE(sparse_matrix_ptrtype& A, vector_ptrtype& F,bool buildCstPart) const = 0;
        virtual void updateBCStrongDirichletLinearPDE(sparse_matrix_ptrtype& A, vector_ptrtype& F) const=0;
        virtual void updateSourceTermLinearPDE(vector_ptrtype& F, bool buildCstPart) const =0;

        //___________________________________________________________________________________//
        //___________________________________________________________________________________//
        // update field from expr
        template < typename ExprT >
        void updateFieldVelocityConvection( vf::Expr<ExprT> const& expr )
        {
            if ( M_fieldVelocityConvection )
                this->updateForUseFunctionSpacesVelocityConvection();
            M_fieldVelocityConvection->on(_range=elements(this->mesh()), _expr=expr );
        }


        //private :
    protected :

        bool M_hasBuildFromMesh, M_isUpdatedForUse;

        mesh_ptrtype M_mesh;

        space_temperature_ptrtype M_Xh;
        element_temperature_ptrtype M_fieldTemperature;
        bool M_fieldVelocityConvectionIsUsed, M_fieldVelocityConvectionIsIncompressible;
        space_velocityconvection_ptrtype M_XhVelocityConvection;
        element_velocityconvection_ptrtype M_fieldVelocityConvection; // only define with convection effect
        bdf_temperature_ptrtype M_bdfTemperature;

        // physical parameter
        double M_thermalConductivity; // [ W/(m*K) ]
        double M_rho; // density [ kg/(m^3) ]
        double M_heatCapacity; // [ J/(kg*K) ]

        // algebraic data/tools
        backend_ptrtype M_backend;
        model_algebraic_factory_ptrtype M_algebraicFactory;
        BlocksBaseVector<double> M_blockVectorSolution;

        export_ptrtype M_exporter;
        bool M_doExportAll, M_doExportVelocityConvection;

        typedef boost::function<void ( vector_ptrtype& F, bool buildCstPart )> updateSourceTermLinearPDE_function_type;
        updateSourceTermLinearPDE_function_type M_overwritemethod_updateSourceTermLinearPDE;

    };

} // namespace FeelModels
} // namespace Feel

#endif /* FEELPP_THERMODYNAMICSBASE_HPP */
