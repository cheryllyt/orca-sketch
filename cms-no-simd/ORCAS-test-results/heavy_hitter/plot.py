import os
import matplotlib.pyplot as plt
import statistics

ARRAY_COUNTERS = [2,3,4,5,6,7]
ARRAY_SIZE = [4,8,16,32,64]
THRESHOLDS = [0.0001, 0.000178, 0.000316, 0.000562, 0.001, 0.00178, 0.00316, 0.00562, 0.01]

COUNTERS_MODE = 0 # plot different array_counter
SIZE_MODE = 1 # plot different array_size

def get_all_values(hh_data, fixed_array_counter=None, fixed_array_size=None):

    count_hh_dict = {}
    size_hh_dict = {}

    for n_buc_counters in ARRAY_COUNTERS:
        count_hh_dict[n_buc_counters] = {}

    for array_size in ARRAY_SIZE:
        size_hh_dict[array_size] = {}

    for line in hh_data:
        split_line = line.split('\t')
        sketch_size = int(split_line[3])
        n_arrays = int(split_line[5])
        n_buc_counters = int(split_line[7])
        rel_error = []
        j = 11
        for threshold in THRESHOLDS:
            rel_error.append(float(split_line[j]))
            j += 4

        array_size = sketch_size / n_arrays

        if (n_buc_counters in count_hh_dict) and (array_size == fixed_array_size):
            if sketch_size not in count_hh_dict[n_buc_counters]:
                count_hh_dict[n_buc_counters][sketch_size] = []
            count_hh_dict[n_buc_counters][sketch_size].append(rel_error)
        
        if (array_size in size_hh_dict) and (n_buc_counters == fixed_array_counter):
            if sketch_size not in size_hh_dict[array_size]:
                size_hh_dict[array_size][sketch_size] = []
            size_hh_dict[array_size][sketch_size].append(rel_error)

    return count_hh_dict, size_hh_dict

def get_median(count_hh_dict_all, size_hh_dict_all):

    count_hh_dict = {}
    size_hh_dict = {}

    for n_buc_counters in ARRAY_COUNTERS:
        count_hh_dict[n_buc_counters] = []

    for n_buc_counters in ARRAY_COUNTERS:
        for sketch_size in count_hh_dict_all[n_buc_counters]:
            meds = []
            n_seeds = len(count_hh_dict_all[n_buc_counters][sketch_size])
            for i in range(len(THRESHOLDS)):
                to_compare = []
                for j in range(n_seeds):
                    to_compare.append(count_hh_dict_all[n_buc_counters][sketch_size][j][i])
                med = statistics.median(to_compare)
                meds.append(med)
            count_hh_dict[n_buc_counters].append((sketch_size, meds))

    for array_size in ARRAY_SIZE:
        size_hh_dict[array_size] = []

    for array_size in ARRAY_SIZE:
        for sketch_size in size_hh_dict_all[array_size]:
            meds = []
            n_seeds = len(size_hh_dict_all[array_size][sketch_size])
            for i in range(len(THRESHOLDS)):
                to_compare = []
                for j in range(n_seeds):
                    to_compare.append(size_hh_dict_all[array_size][sketch_size][j][i])
                med = statistics.median(to_compare)
                meds.append(med)
            size_hh_dict[array_size].append((sketch_size, meds))
    
    return count_hh_dict, size_hh_dict

def plot_speed_and_error(alpha:float, mode:int, fixed_array_counter=None, fixed_array_size=None, fixed_sketch_size=None):
    
    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = '../alpha_' + alpha_str + '/'

    if mode == COUNTERS_MODE:
        folder_name = alpha_folder_name + 'counters_mode/'
    elif mode == SIZE_MODE:
        folder_name = alpha_folder_name + 'size_mode/'

    hh_data = []

    # e.g. alpha_1-0/counters_mode/seed_1_test_orcas_error_on_arrival.txt
    for fn in os.listdir(folder_name):
        indiv_data = open(folder_name + fn).readlines()
        indiv_data = indiv_data[1:] # remove zipf generation line
        if 'final_error' in fn:
            hh_data = hh_data + indiv_data

    count_hh_dict_all, size_hh_dict_all = get_all_values(hh_data, fixed_array_counter, fixed_array_size)
    count_hh_dict, size_hh_dict = get_median(count_hh_dict_all, size_hh_dict_all)

    fig, (hh_ax) = plt.subplots(1, 1)

    if mode == COUNTERS_MODE:
        for n_buc_counters in ARRAY_COUNTERS:
            hh_x = [threshold for threshold in THRESHOLDS]
            hh_y = []
            for (sketch_size, rel_error) in count_hh_dict[n_buc_counters]:
                if sketch_size == fixed_sketch_size:
                    hh_y = rel_error
                    break
            hh_ax.plot(hh_x, hh_y, label=str(n_buc_counters)+' array counters')

    elif mode == SIZE_MODE:
        for array_size in ARRAY_SIZE:
            hh_x = [threshold for threshold in THRESHOLDS]
            hh_y = []
            for (sketch_size, rel_error) in size_hh_dict[array_size]:
                if sketch_size == fixed_sketch_size:
                    hh_y = rel_error
                    break
            hh_ax.plot(hh_x, hh_y, label=str(array_size)+' array size')

    N = hh_data[0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | fixed '
    if fixed_array_counter is not None:
        fig_title = fig_title + 'array counter = ' + str(fixed_array_counter)
    elif fixed_array_size is not None:
        fig_title = fig_title + 'array size = ' + str(fixed_array_size)
    else:
        fig_title = fig_title + '[none]'
    if fixed_sketch_size is not None:
        fig_title = fig_title + ' | fixed sketch size = ' + str(fixed_sketch_size)
    fig.suptitle(fig_title, fontsize=18)

    hh_ax.set_xlabel('Threshold', fontsize=18)
    hh_ax.set_ylabel('ARE', fontsize=18)
    hh_ax.tick_params(axis='both', which='both', labelsize=15)
    hh_ax.set_xscale('log',base=10)
    hh_ax.set_yscale('log',base=10)
    hh_ax.legend(loc="upper right", prop={"size":11})

def plot_all_alpha():

    # plot_speed_and_error(1.0, COUNTERS_MODE, fixed_array_size=16, fixed_sketch_size=1024) # fix array_size at 16 when plotting different array_counter
    # plot_speed_and_error(1.0, COUNTERS_MODE, fixed_array_size=16, fixed_sketch_size=65536)
    # plot_speed_and_error(1.0, COUNTERS_MODE, fixed_array_size=16, fixed_sketch_size=4194304)
    # plot_speed_and_error(1.0, COUNTERS_MODE, fixed_array_size=16, fixed_sketch_size=8388608)

    # plot_speed_and_error(1.0, SIZE_MODE, fixed_array_counter=3, fixed_sketch_size=1024) # fix array_counter at 3 when plotting different array_size
    # plot_speed_and_error(1.0, SIZE_MODE, fixed_array_counter=3, fixed_sketch_size=65536)
    # plot_speed_and_error(1.0, SIZE_MODE, fixed_array_counter=3, fixed_sketch_size=4194304)
    # plot_speed_and_error(1.0, SIZE_MODE, fixed_array_counter=3, fixed_sketch_size=8388608)

    plot_speed_and_error(1.5, SIZE_MODE, fixed_array_counter=3, fixed_sketch_size=65536)
    plot_speed_and_error(1.5, COUNTERS_MODE, fixed_array_size=16, fixed_sketch_size=65536)
    plot_speed_and_error(0.8, SIZE_MODE, fixed_array_counter=3, fixed_sketch_size=65536)
    plot_speed_and_error(0.8, COUNTERS_MODE, fixed_array_size=16, fixed_sketch_size=65536)

    plt.show()

if __name__ == '__main__':
    plot_all_alpha()
