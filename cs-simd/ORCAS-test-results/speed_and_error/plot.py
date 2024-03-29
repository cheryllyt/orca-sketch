import os
import matplotlib.pyplot as plt
import statistics

ARRAY_COUNTERS = [1,3,5]
ARRAY_SIZE = [4,8,16,32,64]

COUNTERS_MODE = 0 # plot different array_counter
SIZE_MODE = 1 # plot different array_size

def get_all_values(speed_data, error_data, fixed_array_counter=None, fixed_array_size=None):

    count_speed_dict = {}
    count_error_dict = {}
    size_speed_dict = {}
    size_error_dict = {}

    for n_buc_counters in ARRAY_COUNTERS:
        count_speed_dict[n_buc_counters] = {}
        count_error_dict[n_buc_counters] = {}

    for array_size in ARRAY_SIZE:
        size_speed_dict[array_size] = {}
        size_error_dict[array_size] = {}

    for line in speed_data:
        split_line = line.split('\t')
        N = int(split_line[1])
        sketch_size = int(split_line[3])
        n_arrays = int(split_line[5])
        n_buc_counters = int(split_line[7])
        time = int(split_line[-1])

        array_size = sketch_size / n_arrays
        speed = N / time

        if (n_buc_counters in count_speed_dict) and (array_size == fixed_array_size):
            if sketch_size not in count_speed_dict[n_buc_counters]:
                count_speed_dict[n_buc_counters][sketch_size] = []
            count_speed_dict[n_buc_counters][sketch_size].append(speed)

        if (array_size in size_speed_dict) and (n_buc_counters == fixed_array_counter):
            if sketch_size not in size_speed_dict[array_size]:
                size_speed_dict[array_size][sketch_size] = []
            size_speed_dict[array_size][sketch_size].append(speed)

    for line in error_data:
        split_line = line.split('\t')
        sketch_size = int(split_line[3])
        n_arrays = int(split_line[5])
        n_buc_counters = int(split_line[7])
        l2 = float(split_line[-3])

        array_size = sketch_size / n_arrays

        if (n_buc_counters in count_error_dict) and (array_size == fixed_array_size):
            if sketch_size not in count_error_dict[n_buc_counters]:
                count_error_dict[n_buc_counters][sketch_size] = []
            count_error_dict[n_buc_counters][sketch_size].append(l2)
        
        if (array_size in size_error_dict) and (n_buc_counters == fixed_array_counter):
            if sketch_size not in size_error_dict[array_size]:
                size_error_dict[array_size][sketch_size] = []
            size_error_dict[array_size][sketch_size].append(l2)
    
    return count_speed_dict, count_error_dict, size_speed_dict, size_error_dict

def get_median(count_speed_dict_all, count_error_dict_all, size_speed_dict_all, size_error_dict_all):

    count_speed_dict = {}
    count_error_dict = {}
    size_speed_dict = {}
    size_error_dict = {}

    for n_buc_counters in ARRAY_COUNTERS:
        count_speed_dict[n_buc_counters] = []
        count_error_dict[n_buc_counters] = []

    for n_buc_counters in ARRAY_COUNTERS:
        for sketch_size in count_speed_dict_all[n_buc_counters]:
            med = statistics.median(count_speed_dict_all[n_buc_counters][sketch_size])
            count_speed_dict[n_buc_counters].append((sketch_size, med))

        for sketch_size in count_error_dict_all[n_buc_counters]:
            med = statistics.median(count_error_dict_all[n_buc_counters][sketch_size])
            count_error_dict[n_buc_counters].append((sketch_size, med))

    for array_size in ARRAY_SIZE:
        size_speed_dict[array_size] = []
        size_error_dict[array_size] = []

    for array_size in ARRAY_SIZE:
        for sketch_size in size_speed_dict_all[array_size]:
            med = statistics.median(size_speed_dict_all[array_size][sketch_size])
            size_speed_dict[array_size].append((sketch_size, med))

        for sketch_size in size_error_dict_all[array_size]:
            med = statistics.median(size_error_dict_all[array_size][sketch_size])
            size_error_dict[array_size].append((sketch_size, med))
    
    return count_speed_dict, count_error_dict, size_speed_dict, size_error_dict

def plot_speed_and_error(alpha:float, mode:int, fixed_array_counter=None, fixed_array_size=None):
    
    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = '../alpha_' + alpha_str + '/'

    speed_data = []
    error_data = []

    # e.g. alpha_1-0/seed_1_test_orcas_error_on_arrival.txt
    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()
        # indiv_data = indiv_data[1:] # remove zipf generation line
        if 'speed' in fn:
            speed_data = speed_data + indiv_data
        elif 'error_on_arrival' in fn:
            error_data = error_data + indiv_data

    count_speed_dict_all, count_error_dict_all, size_speed_dict_all, size_error_dict_all = get_all_values(speed_data, error_data, fixed_array_counter, fixed_array_size)
    count_speed_dict, count_error_dict, size_speed_dict, size_error_dict = get_median(count_speed_dict_all, count_error_dict_all, size_speed_dict_all, size_error_dict_all)

    fig, (speed_ax, error_ax) = plt.subplots(1, 2)

    if mode == COUNTERS_MODE:
        for n_buc_counters in ARRAY_COUNTERS:
            speed_x = [sketch_size*4/1024 for (sketch_size, speed) in count_speed_dict[n_buc_counters]]
            speed_y = [speed for (sketch_size, speed) in count_speed_dict[n_buc_counters]]
            speed_ax.plot(speed_x, speed_y, label=str(n_buc_counters))

            error_x = [sketch_size*4/1024 for (sketch_size, l2) in count_error_dict[n_buc_counters]]
            error_y = [l2 for (sketch_size, l2) in count_error_dict[n_buc_counters]]
            error_ax.plot(error_x, error_y, label=str(n_buc_counters))

    elif mode == SIZE_MODE:
        for array_size in ARRAY_SIZE:
            speed_x = [sketch_size*4/1024 for (sketch_size, speed) in size_speed_dict[array_size]]
            speed_y = [speed for (sketch_size, speed) in size_speed_dict[array_size]]
            speed_ax.plot(speed_x, speed_y, label=str(array_size))

            error_x = [sketch_size*4/1024 for (sketch_size, l2) in size_error_dict[array_size]]
            error_y = [l2 for (sketch_size, l2) in size_error_dict[array_size]]
            error_ax.plot(error_x, error_y, label=str(array_size))

    N = speed_data[0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | fixed '
    if fixed_array_counter is not None:
        fig_title = fig_title + 'array counter = ' + str(fixed_array_counter)
    elif fixed_array_size is not None:
        fig_title = fig_title + 'array size = ' + str(fixed_array_size)
    else:
        fig_title = fig_title + '[none]'
    # fig.suptitle(fig_title, fontsize=18)

    speed_ax.set_xlabel('Memory [KB]', fontsize=22)
    speed_ax.set_ylabel('Throughput [Mops]', fontsize=22)
    speed_ax.tick_params(axis='both', which='both', labelsize=20, width=1, length=4)
    speed_ax.set_xscale('log',base=10)
    # speed_ax.legend(loc="upper right", prop={"size":11})

    error_ax.set_xlabel('Memory [KB]', fontsize=22)
    error_ax.set_ylabel('L2 Error', fontsize=22)
    error_ax.tick_params(axis='both', which='both', labelsize=20, width=1, length=4)
    error_ax.set_xscale('log',base=10)
    error_ax.set_yscale('log',base=10)
    if mode == COUNTERS_MODE:
        error_ax.legend(loc="upper right", prop={"size":20}, ncol=len([count for count in ARRAY_COUNTERS]), bbox_to_anchor=(0.3, 1.14))
    elif mode == SIZE_MODE:
        error_ax.legend(loc="upper right", prop={"size":20}, ncol=len([size for size in ARRAY_SIZE]), bbox_to_anchor=(0.7, 1.14))

def plot_all_alpha():

    # plot_speed_and_error(1.0, COUNTERS_MODE, fixed_array_size=8) # fix array_size at 8 when plotting different array_counter
    # plot_speed_and_error(1.0, SIZE_MODE, fixed_array_counter=3) # fix array_counter at 3 when plotting different array_size

    plot_speed_and_error(1.5, COUNTERS_MODE, fixed_array_size=8)
    plot_speed_and_error(0.8, COUNTERS_MODE, fixed_array_size=8)
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()
