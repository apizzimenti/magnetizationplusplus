
#include <ATEAMS++.h>

#include <SparseRREF/sparse_mat.h>
#include <SparseRREF/sparse_vec.h>

#include <chrono>
#include "metadata.h"

using namespace ATEAMS;
using namespace std;

// For ease-of-use.
typedef statistics::Chain<models::Bernoulli> Chain;

int main(int argc, char* argv[]) {
	// cmd
	int SCALE = std::stoi(argv[1]);
	int TOPDIMENSION = std::stoi(argv[2]);
	int PLAQUETTEDIMENSION = std::stoi(argv[3]);
	string TIMESTAMP = (string)argv[4];

	// Prep metadata.
	METADATA META;
	META.TIMESTAMP = TIMESTAMP;

	// Set the number of iterations here so we know whether we're testing or not.
	int N;

	try {
		META.localize(std::stoi(TIMESTAMP));
		N = 1000000;
	} catch (...) {
		META.localize(TIMESTAMP);
		N = 10;
	}
	

	// Construct a cubical complex.
	vector<int> corners(TOPDIMENSION, SCALE);
	complexes::Cubical C(corners, true);

	// Parametrize + initialize the model.
	models::BernoulliParameters params;
	params.p = 0.5;
	params.dimension = PLAQUETTEDIMENSION;

	models::Bernoulli G(&C, params);

	// Create the chain and data storage buckets.
	Chain M(&G, N);

	vector<float> occupancy(N);
	vector<int> rank(N);

	int t=0;
	auto start = chrono::high_resolution_clock::now();

	for (models::BernoulliState* state : M.simulate<models::BernoulliState>()) {
		occupancy[t] = (double)state->included.size()/(double)C.Cells[params.dimension];
		rank[t] = state->rank;
		t++;

		// Fake a progress bar.
		cout << estimatedTTC(start, t, N);
	}
	cout << endl;

	auto end = chrono::high_resolution_clock::now();

	// Add the parameters we care about to the metadata.
	META.MODEL = G.kind;
	META.PARAMETERS["DENSITY"] = params.p;
	META.PARAMETERS["TOPDIMENSION"] = TOPDIMENSION;
	META.PARAMETERS["PLAQUETTEDIMENSION"] = PLAQUETTEDIMENSION;
	META.PARAMETERS["SCALE"] = SCALE;

	// Localize the time; compute the time-to-completion; write to file.
	META.ttc(start, end);
	META.write(std::format("output/tape/{}/metadata.json", TIMESTAMP));

	// Write data to file.
	ATEAMS::DataWriter writer;

	writer.write(occupancy, std::format("output/tape/{}/{}.txt", TIMESTAMP, "occupancy"));
	writer.write(rank, std::format("output/tape/{}/{}.txt", TIMESTAMP, "rank"));
	
	return 0;
}


