
import numpy as np
import json
from pathlib import Path
import matplotlib.pyplot as plt

from config import histograms

# Turn on latex.
plt.rcParams.update(**histograms.occupation.rcParams)

ROOT = Path("./../../")
STATS = ROOT/"output/statistics"
OUT = ROOT/"output/figures"
TEST = False

if not OUT.exists(): OUT.mkdir()

try:
	with open(ROOT/"completed.txt") as f: STAMPS = [s.strip() for s in f.readlines()]
	assert len(STAMPS) > 0
	assert not TEST
except:
	STAMPS = ["TEST"]

for STAMP in STAMPS:
	with open(ROOT/"output/statistics"/STAMP/"metadata.json") as f: METADATA = json.load(f)

	N = METADATA["iterations"]
	RANK = METADATA["rank"]
	DIM = METADATA["dimension"]
	L = METADATA["scale"][0]

	# Load betti curve data. Get histogram data for each rank.
	giants = np.load(STATS/STAMP/"percentages.npy")
	Ys = histograms.occupation.histograms(giants, RANK)
	X = np.linspace(0, 1, num=2048)
	Zs = histograms.occupation.pdfs(Ys, X)

	rev = list(range(RANK))

	# Create a layered plot with all the histograms on top of each other.
	fig, ax = plt.subplots(figsize=histograms.occupation.figsize)
	colors = histograms.occupation.colors(RANK)
	
	for i in rev:
		Z = Zs[i]/N
		ax.plot(X, Z, lw=1/2, alpha=1/2, color=colors[i])
		ax.fill_between(X, Z, alpha=1/2, color=colors[i], lw=0)

		# Find the x-coordinate of the max value of Y?
		mm = Z.argmax()
		ax.text(X[mm], Z[mm], rf"${i+1}$", fontsize=4, alpha=3/4, ha="right", va="top")
	
	ax.set_xlim(*histograms.occupation.xlim)
	plt.savefig(OUT/histograms.occupation.name(L), **histograms.occupation.savefig)
	