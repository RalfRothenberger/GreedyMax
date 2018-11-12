#include "Snap.h"
#include <set>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <iostream>
using namespace std;

//comparison function for diff, imposes internal order on set, odering elements by second value (difference in function value)
//and tie-breaking with first value (node id)
bool sizeCompare(const std::pair<int, int> a, const std::pair<int, int> b) {
	if (a.second < b.second)
		return true;
	else if (a.second > b.second)
		return false;
	else return (a.first < b.first);
}

int RandomGreedy(PNGraph G, FILE* F, TInt Budget, int s) {
	//seeding random number generator
	std::mt19937 generator;
	if (s == -1)
		generator.seed(time(0));
	else
		generator.seed(s);

	std::set<int> solution, rest;
	//diff counts the difference in solution size each node (not in the solution) contributes for nodes with non-negative difference
	std::set<std::pair<int, int>, bool(*) (const std::pair<int, int> a, const std::pair<int, int> b)> diff(&sizeCompare);
	solution.clear();
	rest.clear();
	diff.clear();
	int solutionSize = 0;
	map<int, int> diffElements;
	diffElements.clear();
	std::set<std::pair<int, int>, bool(*) (const std::pair<int, int> a, const std::pair<int, int> b)> candidates(&sizeCompare);
	candidates.clear();
	for (TNGraph::TNodeI NI = G->BegNI(); NI != G->EndNI(); NI++) {
		rest.insert(NI.GetId());
		diff.insert(make_pair(NI.GetId(), NI.GetOutDeg()));
		diffElements[NI.GetId()] = NI.GetOutDeg();
	}
	for (int k = 0; k < Budget; ++k) {
		int max = 0;
		int argmax = 0;
		candidates.clear();

		//stop if no candidate is left
		if (diff.size() == 0)
			break;
		//the internal ordering of our set imposed by the sizeCompare function implies that the last element of the set contains the
		//node which gives the highest increase in target function, i.e. cut size
		std::set<std::pair<int, int>, bool(*) (const std::pair<int, int> a, const std::pair<int, int> b)>::iterator it = diff.end();
		--it;
		if (it->second < 0)
			break;
		else {
			//uniformly at random choose a node from the Budget many with biggest contribution
			int m = Budget - 1;
			if (diff.size() - 1 < m)
				m = diff.size() - 1;
			std::uniform_int_distribution<int> uni(0, m);
			int r = uni(generator);
			for (; r > 0; --r) {
				--it;
			}
			argmax = it->first;
			max = it->second;
			solutionSize += max;
			diff.erase(*it);

			//move argmax from set of non-solution nodes (rest) to solution set (solution)
			rest.erase(argmax);
			solution.insert(argmax);

			//updating the potential contribtuion of each node to the target function (diff)

			//each neighbor over an outgoing edge which is not part of the solution does now contribute to the cut
			//by adding them, their contirbution will be gone, reducing their increase of the target function by one
			for (int e = 0; e < G->GetNI(argmax).GetOutDeg(); ++e) {
				int outNode = G->GetNI(argmax).GetOutNId(e);
				if (rest.find(outNode) != rest.end()) {
					diff.erase(make_pair(outNode, diffElements[outNode]));
					--diffElements[outNode];
					if (diffElements[outNode] >= 0)
						diff.insert(make_pair(outNode, diffElements[outNode]));
				}
			}
			//each neighbor over an incoming edge loses potential contribution to the target function:
			//before adding the new node (argmax) to the solution, adding any of these neighbors to the solution
			//would have added the edge to argmax to the cut
			for (int e = 0; e < G->GetNI(argmax).GetInDeg(); ++e) {
				int inNode = G->GetNI(argmax).GetInNId(e);
				if (rest.find(inNode) != rest.end()) {
					diff.erase(make_pair(inNode, diffElements[inNode]));
					--diffElements[inNode];
					if (diffElements[inNode] >= 0)
						diff.insert(make_pair(inNode, diffElements[inNode]));
				}
			}
		}



	}
	return solutionSize;
}

int main(int argc, char* argv[]) {
	Env = TEnv(argc, argv, TNotify::StdNotify);
	Env.PrepArgs(TStr::Fmt("Dominating Set. build: %s, %s. Time: %s", __TIME__, __DATE__, TExeTm::GetCurTm()));
	TExeTm ExeTm;
	Try{

		const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "input.txt", "Input directed graph in csv-format, i.e. one directed edge per line.");
		TStr OutFNm = Env.GetIfArgPrefixStr("-o:", ".", "Directory in which to save the output file. The name of the file is determined by the input parameters.");
		const TInt Budget = Env.GetIfArgPrefixInt("-b:", 0, "Integer determining the maximum size of the cut set.");
		const TInt P = Env.GetIfArgPrefixInt("-p:", 100, "Integer determining the maximum size of the cut set as a percentage of the total number of nodes in the graph.");
		const TInt S = Env.GetIfArgPrefixInt("-s:", -1, "Seed of random number generator, -1=system time");
		if (OutFNm.Empty()) { OutFNm = InFNm.GetFMid(); }

		//check if input arguments are inside their bounds
		try {
			if (InFNm == "") throw std::invalid_argument("-i: input not specified");
			if (Budget < 0) throw  std::invalid_argument("-b: value must be non-negative");
			if (P < 0 || P >100) throw std::invalid_argument("-p: value out of bounds, must be integer between 0 and 100");
			if (OutFNm == "") throw std::invalid_argument("-o: output file not specified");
		}
		catch (const std::exception &exc) {
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

		//set output ending correctly, with seed added (for parallelization purposes)
		appendix += "-Rand-";
		appendix += to_string(int(S));
		appendix += ".txt";
		fn.replace(fn.end() - 4, fn.end(), appendix);

		FILE* F = fopen(fn.c_str(), "wt");
		int res = RandomGreedy(G, F, B, S);
		fprintf(F, "%i\n", res);

		fclose(F);
	}
	Catch
		printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
	return 0;
}


