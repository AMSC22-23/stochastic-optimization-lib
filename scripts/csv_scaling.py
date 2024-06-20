# Script that plots the data from the csv files and saves them as png files
# Takes the first column as the x-axis and the rest as the y-axis
# The labels are inferred from the column names on the first row after comments
from glob import glob
import os
import re
import sys
from os import path
import matplotlib
from matplotlib.lines import Line2D
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Function to extract the thread number
def extract_thread(filepath):
	match = re.search(r'_([^_]*)\.csv', filepath)
	if match:
		return int(match.group(1))
	else:
		return None

# If no filename passed, exit
if len(sys.argv) < 2:
	print("Please provide the required plot as an argument.")
	print("Only files in the output folder can be plotted.")
	exit()

# Find the path of CSV file and plots output folder
basepath = path.dirname(__file__)
filepath = path.abspath(path.join(basepath, "..", "output"))
savepath = path.abspath(path.join(basepath, "..", "plots"))

# Create save directory
if not os.path.exists(savepath):
    os.makedirs(savepath)

opt_types = ('saspso', 'abc')
marks = {"saspso": 'o', "abc": 'x'}
# Dataframes for time_numparticles for each optimizer
df_tnp = {}

# Load data into dataframes
for opt in opt_types:
	## time_numparticles data
	file_list = glob(path.join(filepath, "time_numparticles_" + opt + "_*.csv"))
	print("Loading", opt, "files for time_numparticles:")
	for file in file_list: print("\t", file)

	data = []
	# For each time numparticles file, read the data and append it to the list
	for file in file_list:
		df_temp = pd.read_csv(file, comment='#')
		df_temp['threads'] = extract_thread(file)
		data.append(df_temp)

	# Concatenate the dataframes
	df_tnp[opt] = pd.concat(data, ignore_index=True)
	print(df_tnp[opt].info())


# Set the font size
sns.set_theme(font_scale = 1.3)
sns.set_style("darkgrid")


# ====== Plot for strong scaling for each solver ======
#	Plot scaling for all the problem sizes and separately for each of the solvers.
if "strongsingle" in sys.argv[1]:
	print("Plotting strong scaling for each solver")
	# Solve time as a function of the number of processes all in one plot for a single n_dof
	for time_type in ("Parallel_time",):
		for opt in opt_types:
			fig, ax = plt.subplots(figsize=(12, 8))
			df = df_tnp[opt]
			# Sort the data by the number of processes
			df = df.sort_values(by='threads')
			# Get the values of particles and plot them
			for part in df['Num_particles'].drop_duplicates().tolist():
				df1 = df[df['Num_particles'] == part]
				# Compose the label
				lab = opt + " (" + str(part) + " particles)"
				ax.plot(df1['threads'], df1[time_type], label=lab, marker=marks[opt], linestyle='-.', linewidth=2.5, markersize=8)

			#Plot ideal scaling
			proc = df['threads']
			solve = df[time_type]
			ax.plot(proc, 1e4 / proc, label="Ideal scaling", linestyle='--', color='black')
			ax.plot(proc, 1e3 / proc, linestyle='--', color='black')
			ax.set_xlabel("Number of threads")
			ax.set_ylabel(time_type + " time (ms)")
			ax.set_yscale('log')
			ax.set_xscale('log')
			ax.grid(True, which="both", ls="--")
			ax.set_xticks(df['threads'].unique())
			ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
			ax.set_title("Strong scaling of " + time_type + " time for " + opt)
			ax.legend(loc="upper right", fontsize='small', fancybox=True, framealpha=0.5)
			plt.savefig(path.join(savepath, "strong_" + opt + "_" + time_type + ".png"))
			print("Strong scaling plot saved in", path.join(savepath, "strong_" + opt + "_" + time_type + ".png"))

# ====== Plot for parallel speedup ======
if "speedup" in sys.argv[1]:
	print("Plotting parallel speedup for all solvers")
	for time_type in ("Parallel_time",):
		fig, ax = plt.subplots()
		for opt in opt_types:
			df = df_tnp[opt]
			# Sort the data by the number of processes
			df = df.sort_values(by='threads')
			df
			# Get the largest two values of n_dofs and plot them
			for dof_value in df['Num_particles'].drop_duplicates().nlargest(2).tolist():
				df1 = df[df['Num_particles'] == dof_value]
				# Compose the label
				lab = opt + " (" + str(dof_value) + "particles)"
				speedup = df['threads'].min() * df1[time_type].max() / df1[time_type]
				ax.plot(df1['threads'], speedup, label=lab, marker=marks[opt], linestyle='-.', linewidth=2.5, markersize=8)

		# Plot the ideal speedup
		proc = df['threads']
		ax.plot(proc, proc, label="Ideal speedup", linestyle='--', color='black')
		ax.set_xlabel("Number of threads")
		ax.set_ylabel("Speedup")
		ax.set_xscale('log')
		ax.set_yscale('log')
		ax.grid(True, which="both", ls="--")
		ax.set_xticks(df['threads'].unique())
		ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
		ax.set_title("Parallel speedup for all the solvers")
		ax.legend(loc="upper left", fontsize='small', fancybox=True, framealpha=0.5)
		plt.savefig(os.path.join(savepath, "speedup.png"))
		print("Parallel speedup plot saved in", os.path.join(savepath, "speedup.png"))