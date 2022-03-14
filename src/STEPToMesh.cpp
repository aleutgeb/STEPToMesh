//
// Program: STEPToMesh
//
// Description:
//
// The program STEPToMesh converts solids contained in STEP files into triangle meshes.
//
// Copyright(C) 2020 Alexander Leutgeb
//
// This library is free software; you can redistribute it and / or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA
//

#include <TopoDS_Solid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <StlAPI_Writer.hxx>
#include <XSAlgo.hxx>
#include <Interface_Static.hxx>
#include <vector>
#include <cmath>
#include <iostream>
#include "cxxopts.hpp"

struct NamedSolid {
	NamedSolid(const TopoDS_Solid& s, const std::string& n) : solid{s}, name{n} {}

	const TopoDS_Solid solid;
	const std::string  name;
};

void getNamedSolids(const TopLoc_Location& location, const std::string& prefix, unsigned int& id, const Handle(XCAFDoc_ShapeTool) shapeTool,
		const TDF_Label label, std::vector<NamedSolid>& namedSolids) {
	TDF_Label referredLabel{label};
	if (shapeTool->IsReference(label)) shapeTool->GetReferredShape(label, referredLabel);
	std::string name;
	Handle(TDataStd_Name) shapeName;
	if (referredLabel.FindAttribute(TDataStd_Name::GetID(), shapeName)) name = TCollection_AsciiString(shapeName->Get()).ToCString();
	if (name == "") name = std::to_string(id++);
	std::string fullName{prefix + "/" + name};

	TopLoc_Location localLocation = location * shapeTool->GetLocation(label);
	TDF_LabelSequence components;
	if (shapeTool->GetComponents(referredLabel, components)) {
		for (Standard_Integer compIndex = 1; compIndex <= components.Length(); ++compIndex) {
			getNamedSolids(localLocation, fullName, id, shapeTool, components.Value(compIndex), namedSolids);
		}
	}
	else {
		TopoDS_Shape shape;
		shapeTool->GetShape(referredLabel, shape);
		if (shape.ShapeType() == TopAbs_SOLID) {
			BRepBuilderAPI_Transform transform(shape, localLocation, Standard_True);
			namedSolids.emplace_back(TopoDS::Solid(transform.Shape()), fullName);
		}
	}
}

void read(const std::string& inFile, std::vector<NamedSolid>& namedSolids) {
	Handle(TDocStd_Document) document;
	Handle(XCAFApp_Application) application = XCAFApp_Application::GetApplication();
	application->NewDocument(inFile.c_str(), document);
	STEPCAFControl_Reader reader;
	reader.SetNameMode(true);
	IFSelect_ReturnStatus stat = reader.ReadFile(inFile.c_str());
	if (stat != IFSelect_RetDone || !reader.Transfer(document)) throw std::logic_error{std::string{"Could not read '"} + inFile + "'"};
	Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(document->Main());
	TDF_LabelSequence topLevelShapes;
	shapeTool->GetFreeShapes(topLevelShapes);
	unsigned int id{1};
	for (Standard_Integer iLabel = 1; iLabel <= topLevelShapes.Length(); ++iLabel) {
		getNamedSolids(TopLoc_Location{}, "", id, shapeTool, topLevelShapes.Value(iLabel), namedSolids);
	}
}

void write(const std::string& outFile, const std::vector<NamedSolid>& namedSolids, const std::vector<std::string>& select,
		const double linearDeflection, const double angularDeflection, const std::string& format) {
	TopoDS_Compound compound;
	TopoDS_Builder builder;
	builder.MakeCompound(compound);
	if (select.empty()) {
		for (const auto& namedSolid : namedSolids) builder.Add(compound, namedSolid.solid);
	}
	else {
		for (const auto& sel : select) {
			if (sel != "") {
				if (sel[0] == '/') {
					const auto iter = std::find_if(std::begin(namedSolids), std::end(namedSolids), [&](const auto& namesSolid) { return namesSolid.name == sel; });
					if (iter == std::end(namedSolids)) throw std::logic_error{std::string{"Could not find solid with name '"} + sel + "'"};
					builder.Add(compound, iter->solid);
				}
				else {
					try {
						int index{std::stoi(sel)};
						if (index < 1 || index > namedSolids.size()) throw std::logic_error{std::string{"Index out of range: "} + sel};
						builder.Add(compound, namedSolids[index - 1].solid);
					}
					catch (const std::invalid_argument&) {
						throw std::logic_error{std::string("Invalid index: ") + sel};
					}
				}
			}
		}
	}
	BRepMesh_IncrementalMesh mesh(compound, linearDeflection, Standard_False, (std::acos(-1.0) / 180.0) * angularDeflection, Standard_True);
	StlAPI_Writer writer;
	writer.ASCIIMode() = format == "stl_ascii" ? Standard_True : Standard_False;
	if (!writer.Write(compound, outFile.c_str())) throw std::logic_error{std::string{"Could not write '"} + outFile + "'"};
}

std::string str_toupper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); } );
	return s;
}

int main(int argc, char* argv[]) {
	XSAlgo::Init();
	Standard_Integer unitsStart, unitsEnd;
	Standard_Boolean unitsMatch;
	auto units{Interface_Static::Static("xstep.cascade.unit")};
	units->EnumDef(unitsStart, unitsEnd, unitsMatch);
	std::string unitDesc{"Output unit (one of "};
	for (auto i = unitsStart; i <= unitsEnd; i++) {
		if (i > unitsStart) unitDesc += ", ";
		unitDesc += units->EnumVal(i);
	}
	unitDesc += ")";
	cxxopts::Options options{"STEPToMesh", "STEP to triangle mesh conversion"};
	options.add_options()
			("i,in", "Input file", cxxopts::value<std::string>())
			("o,out", "Output file", cxxopts::value<std::string>())
			("f,format", "Output file format (stl_bin or stl_ascii)", cxxopts::value<std::string>()->default_value("stl_bin"))
			("c,content", "List content (solids)")
			("s,select", "Select solids by name or index (comma separated list, index starts with 1)", cxxopts::value<std::vector<std::string>>())
			("l,linear", "Linear deflection", cxxopts::value<double>())
			("a,angular", "Angular deflection (degrees)", cxxopts::value<double>())
			("u,unit", unitDesc, cxxopts::value<std::string>()->default_value("MM"))
			("h,help", "Print usage");
	try {
		const auto result = options.parse(argc, argv);
		if (result.count("unit")) {
			const auto unit{str_toupper(result["unit"].as<std::string>())};
			if (!units->SetCStringValue(unit.c_str())) throw std::logic_error{std::string{"Could not set unit '"} + unit + "'"};
		}
		if (result.count("content")) {
			if (result.count("in")) {
				const std::string inFile = result["in"].as<std::string>();
				std::vector<NamedSolid> namedSolids;
				read(inFile, namedSolids);
				for (const auto& namedSolid : namedSolids) std::cout << namedSolid.name << std::endl;
			}
			else throw std::logic_error{std::string{"Missing option 'in'"}};
		}
		else if (result.count("in") && result.count("out")) {
			const auto inFile{result["in"].as<std::string>()}, outFile{result["out"].as<std::string>()};
			if (!result.count("linear")) throw std::logic_error{std::string{"Missing option 'linear'"}};
			if (!result.count("angular")) throw std::logic_error{std::string{"Missing option 'angular'"}};
			const auto format{result["format"].as<std::string>()};
			if (format != "stl_bin" && format != "stl_ascii") throw std::logic_error{std::string{"Format '"} + format + "' not supported"};
			const auto linearDeflection{result["linear"].as<double>()}, angularDeflection{result["angular"].as<double>()};
			std::vector<std::string> select;
			if (result.count("select")) select = result["select"].as<std::vector<std::string>>();
			std::vector<NamedSolid> namedSolids;
			read(inFile, namedSolids);
			write(outFile, namedSolids, select, linearDeflection, angularDeflection, format);
		}
		else std::cout << options.help() << std::endl;
		return EXIT_SUCCESS;
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what();
		return EXIT_FAILURE;
	}
}
