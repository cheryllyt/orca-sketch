import os
import matplotlib.pyplot as plt
import statistics

ROW_NUMBERS = [1,3,5,7,9]

def get_all_values(speed_data, error_data):

    speed_dict = {}
    error_dict = {}

    for r in ROW_NUMBERS:
        speed_dict[r] = {}
        error_dict[r] = {}

    for line in speed_data:
        split_line = line.split('\t')
        N = int(split_line[1])
        w = int(split_line[3])
        r = int(split_line[5])
        time = int(split_line[-1])

        if r in speed_dict: # safety measure
            if w not in speed_dict[r]: # create column key and array
                speed_dict[r][w] = []
            speed_dict[r][w].append(N/time)

    for line in error_data:
        split_line = line.split('\t')
        w = int(split_line[3])
        r = int(split_line[5])
        l2 = float(split_line[9])

        if r in error_dict:
            if w not in error_dict[r]:
                error_dict[r][w] = []
            error_dict[r][w].append(l2)

    return speed_dict, error_dict

def get_median(speed_dict_all, error_dict_all):

    speed_dict = {}
    error_dict = {}

    for r in ROW_NUMBERS:
        speed_dict[r] = []
        error_dict[r] = []

    for r in ROW_NUMBERS:
        for w in speed_dict_all[r]:
            med = statistics.median(speed_dict_all[r][w])
            speed_dict[r].append((w, med))
        
        for w in error_dict_all[r]:
            med = statistics.median(error_dict_all[r][w])
            error_dict[r].append((w, med))

    return speed_dict, error_dict

def plot_speed_and_error(alpha:float):

    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = 'alpha_' + alpha_str + '/'

    speed_data = []
    error_data = []

    # e.g. alpha_1-0/test_baseline_count_sketch_error_on_arrival_1.txt
    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()
        if 'speed' in fn:
            speed_data = speed_data + indiv_data
        elif 'error_on_arrival' in fn:
            error_data = error_data + indiv_data

    speed_dict_all, error_dict_all = get_all_values(speed_data, error_data)
    speed_dict, error_dict = get_median(speed_dict_all, error_dict_all)

    fig, (speed_ax, error_ax) = plt.subplots(1, 2)

    for r in ROW_NUMBERS:
        speed_x = [r*w*4/1024 for (w, speed) in speed_dict[r]]
        speed_y = [speed for (w, speed) in speed_dict[r]]
        speed_ax.plot(speed_x, speed_y, label=str(r)+' rows')

        error_x = [r*w*4/1024 for (w, l2) in error_dict[r]]
        error_y = [l2 for (w, l2) in error_dict[r]]
        error_ax.plot(error_x, error_y, label=str(r)+' rows')

    N = speed_data[0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha)
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
    
    plot_speed_and_error(1.0)
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()
