
#include <ATEAMS++.h>

#include <SparseRREF/sparse_mat.h>
#include <SparseRREF/sparse_vec.h>

#include <chrono>
#include "metadata.h"

using namespace ATEAMS;
using namespace std;

using Model = models::Bernoulli;
using Parameters = models::ModelParameters;

using Cubical = complexes::Cubical<Model::RingType>;
using State = models::ModelState<Model::RingType,Model::VectorType>;
using Chain = statistics::Chain<Model::RingType,Model::VectorType>;

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
	Cubical COMPLEX(corners, true);

	// Parametrize + initialize the model.
	Parameters PARAMETERS;
	PARAMETERS.p = 0.5;
	PARAMETERS.dimension = PLAQUETTEDIMENSION;

	Model MODEL(&COMPLEX, PARAMETERS);
	Chain CHAIN(&MODEL, N);

	// Data storage.
	vector<float> occupancy(N);
	vector<int> rank(N);

	int t=0;
	auto start = chrono::high_resolution_clock::now();

	for (State STATE : CHAIN.simulate()) {
		occupancy[t] = (double)STATE.includes.size()/(double)COMPLEX.Cells[PARAMETERS.dimension];
		rank[t] = STATE.rank;
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
	META.MODEL = MODEL.name;
	META.PARAMETERS["DENSITY"] = PARAMETERS.p;
	META.PARAMETERS["TOPDIMENSION"] = TOPDIMENSION;
	META.PARAMETERS["PLAQUETTEDIMENSION"] = PLAQUETTEDIMENSION;
	META.PARAMETERS["SCALE"] = SCALE;

	// Localize the time; compute the time-to-completion; write to file.
	META.ttc(start, end);
	META.write(std::format("output/tape/{}/metadata.json", TIMESTAMP));
	
	return 0;
}


