
import numpy as np
import json
from pathlib import Path
import matplotlib.pyplot as plt
from ateams.statistics import cutoffs

from config import autocorrelation
CONFIG = autocorrelation.decay

ROOT = Path("./../../")
STATS = ROOT/"output/statistics"
OUT = ROOT/"output/figures"
with open(ROOT/"completed.txt") as f: STAMPS = [s.strip() for s in f.readlines()]

statistics = ["energy", "occupancy"]

plt.rcParams.update(CONFIG.rcParams)

for STAMP in STAMPS:
	with open(STATS/STAMP/"metadata.json") as f: METADATA = json.load(f)
	L = METADATA["scale"][0]
	c = 8
	STOP = 1000

	for statistic in statistics:
		normalized = np.load(STATS/STAMP/f"{statistic}.autocorrelation.normalized.npy")
		integrated = np.load(STATS/STAMP/f"{statistic}.autocorrelation.integrated.npy")
		M = cutoffs.rectangular(integrated, c=c)
		X = np.arange(integrated.shape[0])

		Z = max(M*10, STOP)

		# Plot; mark the point at which we expect random walk-y behavior to start.
		fig, ax = plt.subplots(figsize=CONFIG.figsize)
		ax.plot(X[:Z], c*integrated[:Z], **CONFIG.plot)
		# ax.scatter(M, c*integrated[M], **CONFIG.scatter)
		ax.axvspan(M, Z, **CONFIG.axvspan)

		ax.set_xlim(1, Z)
		ax.set_xscale("log")
		plt.savefig(OUT/CONFIG.name(statistic, L), **CONFIG.savefig)
