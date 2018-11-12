#include "Snap.h"
#include <map>
#include "gurobi_c++.h"
#include <stdexcept>
using namespace std;

//LP solution
double LP(PNGraph G, FILE* F, TInt Budget) {
	TInt out;
	try {
		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		// Create variables
		GRBVar* x = model.addVars(G->GetNodes(), GRB_CONTINUOUS);
		GRBVar* y = model.addVars(G->GetEdges(), GRB_CONTINUOUS);

		// Integrate new variables and set objective to maximize (=-1)
		model.set(GRB_IntAttr_ModelSense, -1);
		model.update();

		//IDs of networks are non-consecutive, so we need a bijection to the natural numbers from 0 to (n-1)
		map<long int, long int> NodeIdToIndex;
		map<long int, long int> NodeIndexToId;

		int i = 0;
		for (TNGraph::TNodeI NI = G->BegNI(); NI != G->EndNI(); NI++) {
			NodeIdToIndex[NI.GetId()] = i;
			NodeIndexToId[i] = NI.GetId();
			x[i].set(GRB_DoubleAttr_UB, 1.0);
			x[i].set(GRB_DoubleAttr_LB, 0.0);
			x[i].set(GRB_DoubleAttr_Obj, 0.0);
			++i;
		}

		i = 0;
		for (TNGraph::TEdgeI EI = G->BegEI(); EI != G->EndEI(); EI++) {
			y[i].set(GRB_DoubleAttr_UB, 1.0);
			y[i].set(GRB_DoubleAttr_LB, 0.0);
			y[i].set(GRB_DoubleAttr_Obj, 1.0);
			++i;
		}

		TNGraph::TEdgeI EI = G->BegEI();

		i = 0;
		//for each edge, at least one end-point has to be in the VC
		for (EI = G->BegEI(); EI != G->EndEI(); EI++) {
			model.addConstr(y[i], GRB_LESS_EQUAL, 0.5 * x[NodeIdToIndex[EI.GetSrcNId()]] - 0.5 * x[NodeIdToIndex[EI.GetDstNId()]] + 0.5);
			++i;
		}

		//constraint on subset size
		GRBLinExpr expr = 0;
		for (i = 0; i < G->GetNodes(); ++i) {
			expr += x[i];
		}
		model.addConstr(expr, GRB_LESS_EQUAL, Budget);

		// Optimize model
		model.update();
		model.optimize();
		double sum = 0;
		return model.get(GRB_DoubleAttr_ObjVal);
	}
	catch (GRBException e)
	{
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...)
	{
		cout << "Exception during optimization" << endl;
	}
	return -1;
}

//ILP solution
double ILP(PNGraph G, FILE* F, TInt Budget) {
	TInt out;
	try {
		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		// Create variables
		GRBVar* x = model.addVars(G->GetNodes(), GRB_BINARY);
		GRBVar* y = model.addVars(G->GetEdges(), GRB_BINARY);

		// Integrate new variables and set objective to maximize (=-1)
		model.set(GRB_IntAttr_ModelSense, -1);
		model.update();

		//IDs of networks are non-consecutive, so we need a bijection to the natural numbers from 0 to (n-1)
		map<long int, long int> NodeIdToIndex;
		map<long int, long int> NodeIndexToId;

		int i = 0;
		for (TNGraph::TNodeI NI = G->BegNI(); NI != G->EndNI(); NI++) {
			NodeIdToIndex[NI.GetId()] = i;
			NodeIndexToId[i] = NI.GetId();
			x[i].set(GRB_DoubleAttr_UB, 1.0);
			x[i].set(GRB_DoubleAttr_LB, 0.0);
			x[i].set(GRB_DoubleAttr_Obj, 0.0);
			++i;
		}

		i = 0;
		for (TNGraph::TEdgeI EI = G->BegEI(); EI != G->EndEI(); EI++) {
			y[i].set(GRB_DoubleAttr_UB, 1.0);
			y[i].set(GRB_DoubleAttr_LB, 0.0);
			y[i].set(GRB_DoubleAttr_Obj, 1.0);
			++i;
		}

		TNGraph::TEdgeI EI = G->BegEI();

		//for each edge, at least one end-point has to be in the VC
		i = 0;
		for (EI = G->BegEI(); EI != G->EndEI(); EI++) {
			model.addConstr(y[i], GRB_LESS_EQUAL, 0.5 * x[NodeIdToIndex[EI.GetSrcNId()]] - 0.5 * x[NodeIdToIndex[EI.GetDstNId()]] + 0.5);
			++i;
		}

		//constraint on subset size
		GRBLinExpr expr = 0;
		for (i = 0; i < G->GetNodes(); ++i) {
			expr += x[i];
		}
		model.addConstr(expr, GRB_LESS_EQUAL, Budget);

		// Optimize model
		model.update();
		model.optimize();
		return model.get(GRB_DoubleAttr_ObjVal);
	}
	catch (GRBException e)
	{
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...)
	{
		cout << "Exception during optimization" << endl;
	}
	return -1;
}

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("Dominating Set. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try{

		const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "input.txt", "Input directed graph in csv-format, i.e. one directed edge per line.");
		TStr OutFNm = Env.GetIfArgPrefixStr("-o:", "output.txt", "Directory in which to save the output file. The name of the file is determined by the input parameters.");
		const TInt Budget = Env.GetIfArgPrefixInt("-b:", 0, "Integer determining the maximum size of the cut set.");
		const TInt P = Env.GetIfArgPrefixInt("-p:", 100, "Integer determining the maximum size of the cut set as a percentage of the total number of nodes in the graph.");
		const TInt Algo = Env.GetIfArgPrefixInt("-a:", 1, "Choosing between the relaxed and the integer linear program, 0=LP, 1=ILP.");
		if (OutFNm.Empty()) { OutFNm = InFNm.GetFMid(); }

		try {
			//check if input arguments are inside their bounds
			if (InFNm == "") throw std::invalid_argument("-i: input not specified\n");
			if (Algo < 0 || Algo >1) throw std::invalid_argument("-a: can only be 0 or 1\n");
			if (Budget < 0) throw  std::invalid_argument("-b: value must be non-negative\n");
			if (P < 0 || P >100) throw std::invalid_argument("-p: value out of bounds, must be integer between 0 and 100\n");
			if (OutFNm == "") throw std::invalid_argument("-o: output file not specified\n");
		}catch (const std::exception &exc){
			std::cerr << "ERROR:" << exc.what();
			return 0;
		}

		// load graph
		PNGraph G = TSnap::LoadEdgeList<PNGraph>(InFNm, 0, 1);
		for (TNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI++) {
			if (G->IsEdge(NI.GetId(), NI.GetId()))
				G->DelEdge(NI.GetId(), NI.GetId());
		}
		// extract input file name for output
		string fn = InFNm.CStr();
		string out = OutFNm.CStr();
		size_t found = fn.find("/");
		while (found != string::npos) {
			fn.erase(0, found + 1);
			found = fn.find("/");
		}
		fn = out + fn;
		string appendix = "";

		if (P > 0) {
			appendix += "-P-";
			appendix += to_string(int(P));
		}
		else {
			appendix += "-B-";
			appendix += to_string(int(Budget));
		}

		//set budget or relative budget accordingly
		int B = Budget;
		if (P == 100) {
			B = G->GetNodes();
		}
		else if (P > 0) {
			B = G->GetNodes()*P / 100;
		}

		//choose algo
		if (Algo == 0) {
			//set output ending correctly
			appendix += "-LP.txt";
			fn.replace(fn.end() - 4, fn.end(), appendix);

			FILE* F = fopen(fn.c_str(), "wt");
			double sol = ILP(G, F, B);
			fprintf(F, "%f\n", sol);

			fclose(F);
		}
		else if (Algo == 1) {
			//set output ending correctly
			appendix += "-ILP.txt";
			fn.replace(fn.end() - 4, fn.end(), appendix);

			FILE* F = fopen(fn.c_str(), "wt");
			double sol = ILP(G, F, B);
			fprintf(F, "%f\n", sol);

			fclose(F);
		}
	}
	Catch
		printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}


