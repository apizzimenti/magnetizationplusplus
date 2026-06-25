
import numpy as np
import json
from pathlib import Path
import matplotlib.pyplot as plt

# Turn on latex.
plt.rcParams.update({"text.usetex": True})

ROOT = Path("./../../")
STATS = ROOT/"output/statistics"
OUT = ROOT/"output/figures/occupation.decay"
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
	RANK = METADATA["rank"]//2
	q = METADATA["field"]
	z = 1/2

	# Load occupation data.
	OCCUPATION = np.load(STATS/STAMP/"occupation.npy").reshape((-1,N))[:,1:N+1]
	levels = OCCUPATION.shape[0]

	# Create blank plots, set aspect ratio, limits, etc.
	fig, ax = plt.subplots(figsize=(5,3))
	ax.set_ylim(0.48, 0.52)
	ax.set_xlim(0, N)
	ax.spines[["right", "top"]].set_visible(False)

	# Set hline for critical point, plot the average.
	ax.hlines(z, xmin=0, xmax=N, ls=":", color="k", alpha=1/2)

	# Set default plot styles.
	default = dict(lw=2, color="k")

	for level in range(levels):
		ax.plot(
			OCCUPATION[level],
			alpha=1 if RANK <= level+1 <= RANK+1 else 1/4,
			**default
		)

		# Add text for the rank.
		ax.text(
			N+N*0.005, OCCUPATION[level][-1], fr"$\mathbf{{{level+1}}}$",
			alpha=1 if RANK <= level+1 <= RANK+1 else 1/4,
			ha="left", va="center", fontsize=8
		)

	# Re-do ticks?
	xticks = ax.get_xticks()
	ax.set_xticks(xticks)
	ax.set_xticklabels([rf"${str(int(t))}$" for t in xticks])

	yticks = list(ax.get_yticks()) + [z]
	ax.set_yticks(yticks)
	ax.set_yticklabels([rf"${t:.2f}$" for t in yticks[:-1]] + [r"$p_{\mathrm{sd}}$"])

	plt.savefig(OUT/f"{METADATA['scale'][0]}.jpeg", bbox_inches="tight", dpi=1200)

