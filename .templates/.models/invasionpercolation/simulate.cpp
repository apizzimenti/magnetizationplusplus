
#include <ATEAMS++.h>

#include <SparseRREF/sparse_mat.h>
#include <SparseRREF/sparse_vec.h>

#include <chrono>
#include "metadata.h"

using namespace ATEAMS;
using namespace std;

// For ease-of-use.
typedef statistics::Chain<models::InvasionPercolation> Chain;

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
	models::InvasionPercolationParameters params;
	params.field = 2;
	params.stoppingFunction = arithmetic::stopInvadingAt({3,4});

	models::InvasionPercolation G(&C, params);

	// Create the chain and data storage buckets.
	Chain M(&G, N);

	vector<float> occupancy(N);
	vector<int> rank(N);

	int t=0;
	auto start = chrono::high_resolution_clock::now();

	for (models::InvasionPercolationState* state : M.simulate<models::InvasionPercolationState>()) {
		occupancy[t] = (double)state->includes.size()/(double)C.Cells[params.dimension];
		rank[t] = state->rank;
		t++;

		// Fake a progress bar.
		cout << estimatedTTC(start, t, N);
	}
	cout << endl;

	auto end = chrono::high_resolution_clock::now();

	// Write data to file FIRST, since the simulation manager looks for the
	// metadata file before compressing; otherwise, the script errors, since
	// the files are so big.
	ATEAMS::DataWriter writer;

	writer.write(occupancy, std::format("output/tape/{}/{}.txt", TIMESTAMP, "occupancy"));
	writer.write(rank, std::format("output/tape/{}/{}.txt", TIMESTAMP, "rank"));

	// Add the parameters we care about to the metadata.
	META.MODEL = G.kind;
	META.PARAMETERS["DENSITY"] = params.p;
	META.PARAMETERS["TOPDIMENSION"] = TOPDIMENSION;
	META.PARAMETERS["PLAQUETTEDIMENSION"] = PLAQUETTEDIMENSION;
	META.PARAMETERS["SCALE"] = SCALE;

	// Localize the time; compute the time-to-completion; write to file.
	META.ttc(start, end);
	META.write(std::format("output/tape/{}/metadata.json", TIMESTAMP));
	
	return 0;
}


