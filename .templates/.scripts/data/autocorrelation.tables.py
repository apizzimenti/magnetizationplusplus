
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import sys, json
from pathlib import Path

ROOT = Path("../../")
with open(ROOT/"completed.txt") as f: STAMPS = [s.strip() for s in f.readlines()]

# Set the scaling constant for determining the optimal rectangular cutoff.
c = 6

def cutoff(statistic):
	M = 1
	while M < c*statistic[M]: M += 1
	return M

def error(statistic, M):
	return np.sqrt((2*(2*M+1))/statistic.shape[0] * statistic[M]**2)


# Create the tables....
RECORDS = []

for STAMP in STAMPS:
	STATS = ROOT/"output/statistics"/STAMP

	with open(STATS/"metadata.json") as f: METADATA = json.load(f)

	statistics = ["occupancy", "energy"]

	record = {}
	record["L"] = METADATA["scale"][0]
	record["N"] = METADATA["iterations"]
	record["EFF"] = METADATA["iterations"]-METADATA["burn"]

	for statistic in statistics:
		stat = np.load(STATS/f"{statistic}.autocorrelation.integrated.npy")
		M = cutoff(stat)
		err = error(stat, M)

		record[statistic] = stat[M]
		record[f"{statistic}-err"] = np.sqrt(err)

	RECORDS.append(record)

# Write to CSV and to table.
table = pd.DataFrame.from_records(RECORDS)
table["L"] = table["L"]-1
table.to_csv("autocorrelation.tables.csv", index=False)

drop = ["N", "EFF"]

renamer = {
	"L": r"$L$",
	"occupancy": r"$\tau_{\mathcal N}$",
	"occupancy-gd": r"$\tau_{\mathcal N}$ \cite{ossola-sokal}",
	"energy": r"$\tau_{\mathcal E}$",
	"energy-gd": r"$\tau_{\mathcal E}$ \cite{ossola-sokal}"
}

strtran = lambda f: rf"{f:.4f}"

ground = pd.read_csv("autocorrelation.ground.csv")
ground = ground.set_index("L")

table = table.set_index("L")
table = table.sort_index()
table = table.merge(ground, on="L")

columns = ["occupancy", "energy"]
err = [c for c in list(table) if "err" in c]
tags = ["", "-gd"]
ordered = [f"{c}{t}" for c in columns for t in tags]

for c in columns:
	for t in tags:
		table[f"{c}{t}"] = "$" + table[f"{c}{t}"].apply(strtran) + r" \pm " + table[f"{c}-err{t}"].apply(strtran) + "$"


table = table.drop(drop+err, axis=1)
table = table[ordered]
table = table.reset_index()
table["L"] = "$" + table["L"].astype(str) + "$"

table = table.rename(renamer, axis=1)


with open("autocorrelation.tex", "w") as w:
	style = table.style
	style.hide(axis="index")
	writer = style.to_latex
	writer(w, hrules=False, column_format="c|c>{\columncolor[gray]{0.8}}c|c>{\columncolor[gray]{0.8}}c")

