
## What's this repository for?

Creating a streamlined workflow for designing and executing experiments using [ATEAMS++](https://github.com/apizzimenti/ATEAMSplusplus) on remote machines — specifically, [GMU ORC's Hopper high-performance compute cluster](https://wiki.orc.gmu.edu/mkdocs/Hopper_Quick_Start_Guide/), our Lab's Pangolin workstation, or [MEGL](https://megl.science.gmu.edu/)'s MEGLtower workstation. **This workflow is currently compatible with ATEAMS++ version 1.7.**

## How do I use it?

0. Verify whether you have permission to access the remote computing resource.

	* <del>**Hopper:** attempt to log in using the procedure described in the Hopper documentation.</del> **Currently, we only support Pangolin and MEGLtower.**
	* **Pangolin:** attempt to log in via `$ ssh <username>@pangolin.cos.gmu.edu`, where `<username>` is your *Pangolin* username (which should just be your first name).
	* **MEGLtower:** attempt to log in via `$ ssh <username>@megltower.mesa.gmu.edu`, where `<username>` is your *MEGLtower* username (which you will need to request from the MEGL manager).

	After you confirm access, **[create an SSH keypair for your remote host](https://cloud.ibm.com/docs/ssh-keys?topic=ssh-keys-generating-and-using-ssh-keys-for-remote-host-authentication).** We recommend using the naming pattern `gmu-<host>` (e.g. `gmu-pangolin`) for these keys.

1. **Clone the repo** to your local machine by
	```zsh
	$ git clone https://github.com/apizzimenti/magnetizationplusplus
	```

2. **Configure your access file.** Whenever you create a new experiment, the workflow makes copies of the `.<remote>` and `.<remote>.ignore` files that respectively specify how to log into the remote machine and which files to ignore when exchanging files with it. Your `.<remote>` file should read like

	```
	REMOTEUSER=anthony					(username on remote host)
	REMOTELOCATION=gmu-pangolin			(ssh keyname for remote host)
	REMOTEHOST=pangolin					(remote host name)
	USEREMAIL=apizzime					(GMU email prefix, on Hopper only)
	REMOTEROOT="~"						(remote experiments directory prefix)
	```

3. **Create your experiment.** Currently, this workflow supports the following [ATEAMS++ models](https://apizzimenti.github.io/ATEAMSplusplus/namespace_a_t_e_a_m_s_1_1models.html):

	* [`InvadedCluster`](https://apizzimenti.github.io/ATEAMSplusplus/class_a_t_e_a_m_s_1_1models_1_1_invaded_cluster.html)
	* [`SwendsenWang`](https://apizzimenti.github.io/ATEAMSplusplus/class_a_t_e_a_m_s_1_1models_1_1_swendsen_wang.html)
	* [`Invasion`](https://apizzimenti.github.io/ATEAMSplusplus/class_a_t_e_a_m_s_1_1models_1_1_invasion.html)
	* [`Bernoulli`](https://apizzimenti.github.io/ATEAMSplusplus/class_a_t_e_a_m_s_1_1models_1_1_bernoulli.html)
	* [`Glauber`](https://apizzimenti.github.io/ATEAMSplusplus/class_a_t_e_a_m_s_1_1models_1_1_glauber.html)

	To create an experiment, navigate to the `magnetization` directory and run `./experiment <name> <model>`. For example, creating an experiment called `test` using the (e.g.) `InvadedCluster` ATEAMS++ model looks like

	```zsh
	$ ./experiment.sh test InvadedCluster
	```

	This creates an `experiments++` directory (if it doesn't already exist) and an `experiments++/test` directory containing template files for conducting experiments with the `InvadedCluster` algorithm.

4. **Test your experiment.**
	1. **To test the simulation,** navigate to the `experiments++/test` directory and run the `make test` recipe, which builds the default `simulate.cpp` file and executes `./simulate 4 4 2 TEST`, which simulates a 10-sample Markov chain using the [plaquette invaded-cluster algorithm](https://arxiv.org/abs/2507.13503) on the $`2`$-subcomplexes of $\mathbb T^4_4$, the scale-$`4`$ four-torus. **If the last argument to `./simulate` is any nonnegative integer, it will perform a length-$`10^6`$ run. This is the default length, and it can be changed in `simulate.cpp`.**
	<!-- 2. **To test the replay and statistic-computation routines,** run the `replay.statistics.py` (and, if applicable, the `replay.autocorrelation.py`) script(s). Doing so creates the `output/statistics/TEST` directory, which includes an updated `metadata.json` and compressed statistical data. -->

5. **Configure your experiment.** The `simulation.pangolin.sh` file executes (a configurable number of repetitions of) your experiment at varying lattice scales. At the top of the `simulation.pangolin.sh` file, you'll find the following variables:

	```zsh
	COPIES=${1:-1}				# copies at each scale; default 1
	SCALES=(3 4 6 8 11 16 23)	# lattice scales
	TOPDIMENSION=4				# dimension of ambient complex/torus
	PLAQUETTEDIMENSION=2		# dimension of sampled subcomplexes
	```

	Change these to suit your needs. In the `simulate.cpp` file, you'll find a parametrization that looks like
	
	```c++
	vector<int> corners(TOPDIMENSION, SCALE);
	Cubical COMPLEX(corners);

	Model::RingType RR(2);

	// Parametrize + initialize the model.
	Parameters PARAMETERS;
	PARAMETERS.coefficients = &RR;
	PARAMETERS.stoppingFunction = statistics::stopInvadingAt({3,4});
	PARAMETERS.dimension = PLAQUETTEDIMENSION;
	```

	For more info on configuring each `Model`, [read the documentation](https://apizzimenti.github.io/ATEAMSplusplus/namespace_a_t_e_a_m_s_1_1models.html); each `Model` has a default set of parameters. **As of now, most `Model`s fail silently; if you forget a parameter that does not have a default value, the `Model` will not warn you.**

	At each iteration of the Markov chain, the `Model` will update a [`ModelState`](https://apizzimenti.github.io/ATEAMSplusplus/struct_a_t_e_a_m_s_1_1models_1_1_model_state.html) object, yielded by the `Chain` at each iteration. [The docs have information on what data is tracked by the `ModelState` object](https://apizzimenti.github.io/ATEAMSplusplus/struct_a_t_e_a_m_s_1_1models_1_1_model_state.html). However, the `Chain` does _not_ store the data for you: you'll have to keep track of it yourself. For example, the invaded-cluster model keeps track of the current cochain (spin configuration) and the indices of $`d`$-cells included in the current subcomplex in the `cochain` and `includes` properties, respectively. The default strategy for storing these data is to either keep them in a sparse/compressed format (e.g. a `SparseMatrix` for storing cochains of type `SparseVector`), or to transform them and store a smaller representation:

	```c++
	SparseMatrix<Model::RingType> spins(N, C.Cells[params.dimension-1]);
	vector<double> occupancy(N);
	int t=0;

	using State = models::ModelState<Model::RingType,Model::VectorType>;

	for (State STATE : M.simulate()) {
		...

		spins.rows[t] = STATE.cochain;	// keep the same type
		occupancy[t] = (double)STATE.includes.size()/(double)C.Cells[params.dimension]; // transform!
		t++;

		...
	}
	```

6. **Run your experiment.** Though you *can* execute all the steps below on your own machine, the workflow is designed for you to set-and-forget your simulations on a remote machine.
	1. **Upload the experiment to Pangolin/MEGLtower.** (If required — as it is for GMU remote computing resources — connect to the VPN.) In the `experiments++/test` directory, run `./update.sh -p` to send a slim copy of these files to the `~/experiments++/test` directory on Pangolin; doing so with `./update.sh -m` instead will upload them to MEGLtower. If you want to send your files to a location other than `~`, change the value of `REMOTEROOT` in your remote configuration files.
	2. **SSH into Pangolin/MEGLtower** and navigate to the directory with your experiment.
	3. **Start the simulation manager** by running `make simulate`. Your `simulate.cpp` code will be re-compiled for use on the remote machine; then, the simulation manager will begin your simulation(s) as background processes using [GNU `screen`](https://linux.die.net/man/1/screen), terminating once the last simulation completes. **After you start the manager, you can completely log out of Pangolin/MEGLtower. Doing so will not halt your simulations.** You can see currently running processes using `screen -ls`, and re-attach to a given process using `screen -r <process name>`. Each simulation's name is `<experiment name>.<timestamp>`, where `<timestamp>` is the epoch time at which the experiment was started. These names are configurable in the `simulation.<remote>.sh` file.

		![Image of screen -ls output.](https://github.com/apizzimenti/magnetizationplusplus/blob/main/.templates/screen-list.png?raw=true)

		The above image shows how currently-running simulations and managers appear on Pangolin/MEGLtower. To rejoin one of these processes, I would execute the

		```zsh
		$ screen -r 4torus-invadedcluster-2.simulation.1782418399
		```

		command, which shows an `<iterations completed> ———— <time elapsed> ——— <estimated ttc>` triplet, as below:

		![Image of screen -r output; time to completion.](https://github.com/apizzimenti/magnetizationplusplus/blob/main/.templates/screen-ttc.png?raw=true)
		
		You can detach from the screen *without stopping the simulation* by inputting ctrl+a+d. Once the simulation completes, its output is written to `output/tape/<timestamp>`, and its timestamp is recorded in `completed.txt` for later use.

	<!-- 4. **Run statistics.** After your simulations complete, log back into Pangolin, navigate to your experiment's directory, and run `./replay.manager.pangolin.sh`. **After you start the manager, you can completely log out of Pangolin. Doing so will not halt the replays.** Much like the simulation manager, the replay manager will replay and compute statistics on all the completed simulations (i.e. all simulations whose timestamps/names are included in the `completed.txt` file) as background processes and write output to `output/statistics`. As before, each simulation has its own subdirectory containing its statistical data and metadata. -->

	* **Notes on performance:** if you think your simulations or replays are taking too long, run the `top` command to see how Pangolin's resources are being allocated to different processes. It's likely that another user is running simulations at the same time, which can significantly gum things up. To kill a `screen` process, run

		```zsh
		$ screen -X -S <process name> kill
		```

		The killed simulation will have partial (and thus, as of now, unrecoverable) recorded data in its corresponding `output/tape/<timestamp>` directory. Data are recorded in `.txt` files, and are compressed upon completion.
<!-- 
7. **Retrieve data from your experiment.** Once your replays are complete, you can download the (compressed) statistical data from Pangolin by navigating to your experiment's directory *on your machine* and running `./retrieve.sh -p`. Doing so copies all data from your experiment's `output/statistics` directory *on Pangolin* to the `output/statistics` directory *on your machine*.

8. **Make pictures (or tables, or whatever).** The Python scripts in the `scripts/figures` and `scripts/data` directories can automatically detect the locations of your statistical data and create pre-fab plots for them. If you want to create other data visualizations, please add them to the appropriate subdirectory of `scripts`, or create a new one to suit your needs. -->
