import os
import matplotlib.pyplot as plt
import statistics

BUCKET_COUNTERS = [2, 3, 4, 5, 6, 7]
BUCKET_SIZE = [4, 8, 16, 32, 64]
ARRAY_SKETCH_SIZE = 1 # array size = sketch size -> number of arrays = 1

COUNTERS_MODE = 0 # plot different bucket_counter
SIZE_MODE = 1 # plot different bucket_size

def get_all_values(speed_data, error_data, fixed_bucket_counter=None, fixed_bucket_size=None):

    count_speed_dict = {}
    count_error_dict = {}
    size_speed_dict = {}
    size_error_dict = {}
    array_sketch_size_speed_dict = {}
    array_sketch_size_error_dict = {}

    for n_buc_counters in BUCKET_COUNTERS:
        count_speed_dict[n_buc_counters] = {}
        count_error_dict[n_buc_counters] = {}

    for bucket_size in BUCKET_SIZE:
        size_speed_dict[bucket_size] = {}
        size_error_dict[bucket_size] = {}
    
    array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE] = {}
    array_sketch_size_error_dict[ARRAY_SKETCH_SIZE] = {}

    for line in speed_data:
        split_line = line.split('\t')
        N = int(split_line[1])
        sketch_size = int(split_line[3])
        n_buckets = int(split_line[5])
        n_buc_counters = int(split_line[7])
        time = int(split_line[-1])

        bucket_size = sketch_size / n_buckets
        speed = N / time

        if (n_buc_counters in count_speed_dict) and (bucket_size == fixed_bucket_size):
            if sketch_size not in count_speed_dict[n_buc_counters]:
                count_speed_dict[n_buc_counters][sketch_size] = []
            count_speed_dict[n_buc_counters][sketch_size].append(speed)

        if (bucket_size in size_speed_dict) and (n_buc_counters == fixed_bucket_counter):
            if sketch_size not in size_speed_dict[bucket_size]:
                size_speed_dict[bucket_size][sketch_size] = []
            size_speed_dict[bucket_size][sketch_size].append(speed)
        elif (n_buckets == ARRAY_SKETCH_SIZE) and (n_buc_counters == fixed_bucket_counter):
            if sketch_size not in array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE]:
                array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE][sketch_size] = []
            array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE][sketch_size].append(speed)

    for line in error_data:
        split_line = line.split('\t')
        sketch_size = int(split_line[3])
        n_buckets = int(split_line[5])
        n_buc_counters = int(split_line[7])
        l2 = float(split_line[-3])

        bucket_size = sketch_size / n_buckets

        if (n_buc_counters in count_error_dict) and (bucket_size == fixed_bucket_size):
            if sketch_size not in count_error_dict[n_buc_counters]:
                count_error_dict[n_buc_counters][sketch_size] = []
            count_error_dict[n_buc_counters][sketch_size].append(l2)
        
        if (bucket_size in size_error_dict) and (n_buc_counters == fixed_bucket_counter):
            if sketch_size not in size_error_dict[bucket_size]:
                size_error_dict[bucket_size][sketch_size] = []
            size_error_dict[bucket_size][sketch_size].append(l2)
        elif (n_buckets == ARRAY_SKETCH_SIZE) and (n_buc_counters == fixed_bucket_counter):
            if sketch_size not in array_sketch_size_error_dict[ARRAY_SKETCH_SIZE]:
                array_sketch_size_error_dict[ARRAY_SKETCH_SIZE][sketch_size] = []
            array_sketch_size_error_dict[ARRAY_SKETCH_SIZE][sketch_size].append(l2)
    
    return count_speed_dict, count_error_dict, size_speed_dict, size_error_dict, array_sketch_size_speed_dict, array_sketch_size_error_dict

def get_median(count_speed_dict_all, count_error_dict_all, size_speed_dict_all, size_error_dict_all, array_sketch_size_speed_dict_all, array_sketch_size_error_dict_all):

    count_speed_dict = {}
    count_error_dict = {}
    size_speed_dict = {}
    size_error_dict = {}
    array_sketch_size_speed_dict = {}
    array_sketch_size_error_dict = {}

    for n_buc_counters in BUCKET_COUNTERS:
        count_speed_dict[n_buc_counters] = []
        count_error_dict[n_buc_counters] = []

    for n_buc_counters in BUCKET_COUNTERS:
        for sketch_size in count_speed_dict_all[n_buc_counters]:
            med = statistics.median(count_speed_dict_all[n_buc_counters][sketch_size])
            count_speed_dict[n_buc_counters].append((sketch_size, med))

        for sketch_size in count_error_dict_all[n_buc_counters]:
            med = statistics.median(count_error_dict_all[n_buc_counters][sketch_size])
            count_error_dict[n_buc_counters].append((sketch_size, med))

    for bucket_size in BUCKET_SIZE:
        size_speed_dict[bucket_size] = []
        size_error_dict[bucket_size] = []

    for bucket_size in BUCKET_SIZE:
        for sketch_size in size_speed_dict_all[bucket_size]:
            med = statistics.median(size_speed_dict_all[bucket_size][sketch_size])
            size_speed_dict[bucket_size].append((sketch_size, med))

        for sketch_size in size_error_dict_all[bucket_size]:
            med = statistics.median(size_error_dict_all[bucket_size][sketch_size])
            size_error_dict[bucket_size].append((sketch_size, med))
    
    array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE] = []
    array_sketch_size_error_dict[ARRAY_SKETCH_SIZE] = []

    for sketch_size in array_sketch_size_speed_dict_all[ARRAY_SKETCH_SIZE]:
        med = statistics.median(array_sketch_size_speed_dict_all[ARRAY_SKETCH_SIZE][sketch_size])
        array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE].append((sketch_size, med))

    for sketch_size in array_sketch_size_error_dict_all[ARRAY_SKETCH_SIZE]:
        med = statistics.median(array_sketch_size_error_dict_all[ARRAY_SKETCH_SIZE][sketch_size])
        array_sketch_size_error_dict[ARRAY_SKETCH_SIZE].append((sketch_size, med))
    
    return count_speed_dict, count_error_dict, size_speed_dict, size_error_dict, array_sketch_size_speed_dict, array_sketch_size_error_dict

def plot_speed_and_error(alpha:float, mode:int, fixed_bucket_counter=None, fixed_bucket_size=None):
    
    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = 'alpha_' + alpha_str + '/'

    if mode == COUNTERS_MODE:
        folder_name = alpha_folder_name + 'counters_mode/'
    elif mode == SIZE_MODE:
        folder_name = alpha_folder_name + 'size_mode/'

    speed_data = []
    error_data = []

    # e.g. alpha_1-0/counters_mode/seed_1_test_orcas_error_on_arrival.txt
    for fn in os.listdir(folder_name):
        indiv_data = open(folder_name + fn).readlines()
        indiv_data = indiv_data[1:] # remove zipf generation line
        if 'speed' in fn:
            speed_data = speed_data + indiv_data
        elif 'error_on_arrival' in fn:
            error_data = error_data + indiv_data

    count_speed_dict_all, count_error_dict_all, size_speed_dict_all, size_error_dict_all, array_sketch_size_speed_dict_all, array_sketch_size_error_dict_all = get_all_values(speed_data, error_data, fixed_bucket_counter, fixed_bucket_size)
    count_speed_dict, count_error_dict, size_speed_dict, size_error_dict, array_sketch_size_speed_dict, array_sketch_size_error_dict = get_median(count_speed_dict_all, count_error_dict_all, size_speed_dict_all, size_error_dict_all, array_sketch_size_speed_dict_all, array_sketch_size_error_dict_all)

    fig, (speed_ax, error_ax) = plt.subplots(1, 2)

    if mode == COUNTERS_MODE:
        for n_buc_counters in BUCKET_COUNTERS:
            speed_x = [sketch_size*4/1024 for (sketch_size, speed) in count_speed_dict[n_buc_counters]]
            speed_y = [speed for (sketch_size, speed) in count_speed_dict[n_buc_counters]]
            speed_ax.plot(speed_x, speed_y, label=str(n_buc_counters)+' array counters')

            error_x = [sketch_size*4/1024 for (sketch_size, l2) in count_error_dict[n_buc_counters]]
            error_y = [l2 for (sketch_size, l2) in count_error_dict[n_buc_counters]]
            error_ax.plot(error_x, error_y, label=str(n_buc_counters)+' array counters')

    elif mode == SIZE_MODE:
        for bucket_size in BUCKET_SIZE:
            speed_x = [sketch_size*4/1024 for (sketch_size, speed) in size_speed_dict[bucket_size]]
            speed_y = [speed for (sketch_size, speed) in size_speed_dict[bucket_size]]
            speed_ax.plot(speed_x, speed_y, label=str(bucket_size)+' array size')

            error_x = [sketch_size*4/1024 for (sketch_size, l2) in size_error_dict[bucket_size]]
            error_y = [l2 for (sketch_size, l2) in size_error_dict[bucket_size]]
            error_ax.plot(error_x, error_y, label=str(bucket_size)+' array size')
        
        speed_x = [sketch_size*4/1024 for (sketch_size, speed) in array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE]]
        speed_y = [speed for (sketch_size, speed) in array_sketch_size_speed_dict[ARRAY_SKETCH_SIZE]]
        speed_ax.plot(speed_x, speed_y, label='array = sketch size')

        error_x = [sketch_size*4/1024 for (sketch_size, l2) in array_sketch_size_error_dict[ARRAY_SKETCH_SIZE]]
        error_y = [l2 for (sketch_size, l2) in array_sketch_size_error_dict[ARRAY_SKETCH_SIZE]]
        error_ax.plot(error_x, error_y, label='array = sketch size')

    N = speed_data[0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | fixed '
    if fixed_bucket_counter is not None:
        fig_title = fig_title + 'array counter = ' + str(fixed_bucket_counter)
    elif fixed_bucket_size is not None:
        fig_title = fig_title + 'array size = ' + str(fixed_bucket_size)
    else:
        fig_title = fig_title + '[none]'
    fig.suptitle(fig_title, fontsize=18)

    speed_ax.set_xlabel('Memory [KB]', fontsize=18)
    speed_ax.set_ylabel('Throughput [Mops]', fontsize=18)
    speed_ax.tick_params(axis='both', which='both', labelsize=15)
    speed_ax.set_xscale('log',base=10)
    speed_ax.legend(loc="upper right", prop={"size":11})

    error_ax.set_xlabel('Memory [KB]', fontsize=18)
    error_ax.set_ylabel('L2 Error', fontsize=18)
    error_ax.tick_params(axis='both', which='both', labelsize=15)
    error_ax.set_xscale('log',base=10)
    error_ax.set_yscale('log',base=10)
    error_ax.legend(loc="upper right", prop={"size":11})

def plot_all_alpha():

    plot_speed_and_error(1.0, COUNTERS_MODE, fixed_bucket_size=8) # fix bucket_size at 8 when plotting different bucket_counter
    plot_speed_and_error(1.0, SIZE_MODE, fixed_bucket_counter=3) # fix bucket_counter at 2 when plotting different bucket_size
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()
