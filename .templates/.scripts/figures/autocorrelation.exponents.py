
import pandas as pd
import numpy as np
import scipy
from scipy.optimize import curve_fit
from pathlib import Path
import matplotlib.pyplot as plt

from config import autocorrelation
CONFIG = autocorrelation.exponents

OUT = Path("./../../output/figures/")
ROOT = Path("./../data/")
auto = pd.read_csv(ROOT/"autocorrelation.tables.csv")

statistics = ["energy", "occupancy"]

# "power-law" fit.
def plaw(X, a, b): return a*np.power(X,b)

def _dplaw(x, a, b): return np.array([
	np.power(x,b),
	a*np.power(x,b)*np.log(x)
])

def dplaw(X, pcov, A, b):
	return np.dot(pcov[:,0], _dplaw(X, A, b))*np.power(X,b) + np.dot(pcov[:,1], _dplaw(X, A, b))*(A*np.power(X,b)*np.log(X))

# Use tex.
plt.rcParams.update(CONFIG.rcParams)

for statistic in statistics:
	X = auto.L
	Y = auto[statistic]

	XX = np.linspace(X[0]-1, X[X.shape[0]-1]+1)

	# Compute fit, errors.
	popt, pcov = curve_fit(plaw, X, Y, maxfev=200)
	A, b = popt
	err = np.sqrt(np.diag(pcov))
	Aerr, berr = err
	law = plaw(XX, A, b)

	# Compute confidence intervals.
	CI = CONFIG.CI
	npcov = pcov/np.dot(err,err)
	Z = -scipy.stats.t.ppf((1-CI)/2, X.shape[0]-2)

	error = Z*dplaw(XX, npcov, A, b)
	lower = law-error
	upper = law+error

	# Create plot.
	fig, ax = plt.subplots(figsize=CONFIG.figsize)

	# Scatter data points; plot fit; plot confidence interval.
	ax.plot(XX, law, **CONFIG.plot)
	ax.fill_between(XX, lower, upper, **CONFIG.fill_between)
	ax.scatter(X, Y, **CONFIG.scatter)

	# Re-scale.
	ax = CONFIG.xscale(ax)
	# ax = CONFIG.yscale(ax)
	ax = CONFIG.xticks(ax, X)

	# Add some text.
	ax.text(0.025,0.925, CONFIG.time(statistic, A, b), **CONFIG.textdefaults(ax))
	ax.text(0.025,0.875, CONFIG.exponent(statistic, b, berr), **CONFIG.textdefaults(ax))
	ax.text(XX[0], upper[0], CONFIG.interval(CI), fontsize=6, va="bottom", alpha=1/2)

	# Plot.
	plt.savefig(OUT/CONFIG.name(statistic), **CONFIG.savefig)

