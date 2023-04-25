import os
import matplotlib.pyplot as plt
import statistics

ORCAS = 'ORCAS'
CS = 'CS'

SKETCHES = [ORCAS, CS]

def get_all_values(speed_data, error_data, fixed_orcas, fixed_cs):

    speed_dict = {}
    error_dict = {}
    
    for sketch in SKETCHES:

        if sketch == ORCAS:
            for array_counter in fixed_orcas:
                key = ORCAS + '-' + str(array_counter)

                speed_dict[key] = {}
                error_dict[key] = {}

                sketch_speed_data = speed_data[sketch]
                for line in sketch_speed_data:
                    split_line = line.split('\t')

                    N = int(split_line[1])
                    sketch_size = int(split_line[3])
                    n_array_counters = int(split_line[7])
                    time = int(split_line[-1])

                    speed = N / time

                    if n_array_counters == array_counter:
                        if sketch_size not in speed_dict[key]:
                            speed_dict[key][sketch_size] = []
                        speed_dict[key][sketch_size].append(speed)
                
                sketch_error_data = error_data[sketch]
                for line in sketch_error_data:
                    split_line = line.split('\t')

                    sketch_size = int(split_line[3])
                    n_array_counters = int(split_line[7])
                    l2 = float(split_line[-3])

                    if n_array_counters == array_counter:
                        if sketch_size not in error_dict[key]:
                            error_dict[key][sketch_size] = []
                        error_dict[key][sketch_size].append(l2)

        elif sketch == CS:
            for row_number in fixed_cs:
                key = CS + '-' + str(row_number)

                speed_dict[key] = {}
                error_dict[key] = {}

                sketch_speed_data = speed_data[sketch]
                for line in sketch_speed_data:
                    split_line = line.split('\t')

                    N = int(split_line[1])
                    width = int(split_line[3])
                    n_rows = int(split_line[5])
                    time = int(split_line[-1])

                    speed = N / time

                    if n_rows == row_number:
                        if width not in speed_dict[key]:
                            speed_dict[key][width] = []
                        speed_dict[key][width].append(speed)

                sketch_error_data = error_data[sketch]
                for line in sketch_error_data:
                    split_line = line.split('\t')

                    width = int(split_line[3])
                    n_rows = int(split_line[5])
                    l2 = float(split_line[9])

                    if n_rows == row_number:
                        if width not in error_dict[key]:
                            error_dict[key][width] = []
                        error_dict[key][width].append(l2)

    return speed_dict, error_dict

def get_median(speed_dict_all, error_dict_all):

    speed_dict = {}
    error_dict = {}

    for key in speed_dict_all:
        speed_dict[key] = []
        error_dict[key] = []

        for size in speed_dict_all[key]:
            med = statistics.median(speed_dict_all[key][size])
            speed_dict[key].append((size, med))
        
        for size in error_dict_all[key]:
            med = statistics.median(error_dict_all[key][size])
            error_dict[key].append((size, med))

    return speed_dict, error_dict

def plot_speed_and_error(alpha:float, fixed_orcas:int, fixed_cs:int): # ORCAS array counters, CS row numbers

    speed_data = {}
    error_data = {}

    for sketch in SKETCHES:
        speed_data[sketch] = []
        error_data[sketch] = []

    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = '../../ORCAS-test-results/alpha_' + alpha_str + '/counters_mode/'

    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()
        if 'speed' in fn:
            speed_data[ORCAS] = speed_data[ORCAS] + indiv_data
        elif 'error_on_arrival' in fn:
            error_data[ORCAS] = error_data[ORCAS] + indiv_data
    
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = '../../../baseline-results/cs/alpha_' + alpha_str + '/'

    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()
        if 'speed' in fn:
            speed_data[CS] = speed_data[CS] + indiv_data
        elif 'error_on_arrival' in fn:
            error_data[CS] = error_data[CS] + indiv_data

    speed_dict_all, error_dict_all = get_all_values(speed_data, error_data, fixed_orcas, fixed_cs)
    speed_dict, error_dict = get_median(speed_dict_all, error_dict_all)

    fig, (speed_ax, error_ax) = plt.subplots(1, 2)

    for key in speed_dict:
        
        if ORCAS in key:
            key_label = key.split('-')[0] + ': ' + key.split('-')[1]

            speed_x = [sketch_size*4/1024 for (sketch_size, speed) in speed_dict[key]]
            speed_y = [speed for (sketch_size, speed) in speed_dict[key]]
            speed_ax.plot(speed_x, speed_y, label=str(key_label))

            error_x = [sketch_size*4/1024 for (sketch_size, speed) in error_dict[key]]
            error_y = [speed for (sketch_size, speed) in error_dict[key]]
            error_ax.plot(error_x, error_y, label=str(key_label))

        elif CS in key:
            row_number = int(key.split('-')[1])
            key_label = key.split('-')[0] + ': ' + key.split('-')[1]

            speed_x = [row_number*width*4/1024 for (width, speed) in speed_dict[key]]
            speed_y = [speed for (width, speed) in speed_dict[key]]
            speed_ax.plot(speed_x, speed_y, label=str(key_label))

            error_x = [row_number*width*4/1024 for (width, l2) in error_dict[key]]
            error_y = [l2 for (width, l2) in error_dict[key]]
            error_ax.plot(error_x, error_y, label=str(key_label))

    N = speed_data[ORCAS][0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | ORCAS config: array counter | CS config: row number'
    # fig.suptitle(fig_title, fontsize=18)

    speed_ax.set_xlabel('Memory [KB]', fontsize=22)
    speed_ax.set_ylabel('Throughput [Mops]', fontsize=22)
    speed_ax.tick_params(axis='both', which='both', labelsize=20, width=1, length=4)
    speed_ax.set_xscale('log',base=10)
    speed_ax.legend(loc="upper right", prop={"size":11})

    error_ax.set_xlabel('Memory [KB]', fontsize=22)
    error_ax.set_ylabel('L2 Error', fontsize=22)
    error_ax.tick_params(axis='both', which='both', labelsize=20, width=1, length=4)
    error_ax.set_xscale('log',base=10)
    error_ax.set_yscale('log',base=10)
    error_ax.legend(loc="upper right", prop={"size":18}, ncol=len([key for key in speed_dict]), bbox_to_anchor=(1.01, 1.14))

def plot_all_alpha():
    
    # plot_speed_and_error(1.0, fixed_orcas=[3,4], fixed_cs=[1,2,3])
    # plot_speed_and_error(1.0, fixed_orcas=[3,4], fixed_cs=[1,2,3,4])
    
    plot_speed_and_error(1.5, fixed_orcas=[1,3,5], fixed_cs=[1,3,5])
    plot_speed_and_error(0.8, fixed_orcas=[1,3,5], fixed_cs=[1,3,5])
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()        
