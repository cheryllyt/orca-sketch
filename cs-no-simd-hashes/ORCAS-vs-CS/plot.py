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
        speed_dict[sketch] = {}
        error_dict[sketch] = {}

        sketch_speed_data = speed_data[sketch]
        for line in sketch_speed_data:
            split_line = line.split('\t')

            if sketch == ORCAS:
                N = int(split_line[1])
                sketch_size = int(split_line[3])
                n_array_counters = int(split_line[7])
                time = int(split_line[-1])

                speed = N / time

                if n_array_counters == fixed_orcas:
                    if sketch_size not in speed_dict[sketch]:
                        speed_dict[sketch][sketch_size] = []
                    speed_dict[sketch][sketch_size].append(speed)

            elif sketch == CS:
                N = int(split_line[1])
                width = int(split_line[3])
                n_rows = int(split_line[5])
                time = int(split_line[-1])

                speed = N / time

                if n_rows == fixed_cs:
                    if width not in speed_dict[sketch]:
                        speed_dict[sketch][width] = []
                    speed_dict[sketch][width].append(speed)

        sketch_error_data = error_data[sketch]
        for line in sketch_error_data:
            split_line = line.split('\t')

            if sketch == ORCAS:
                sketch_size = int(split_line[3])
                n_array_counters = int(split_line[7])
                l2 = float(split_line[-3])

                if n_array_counters == fixed_orcas:
                    if sketch_size not in error_dict[sketch]:
                        error_dict[sketch][sketch_size] = []
                    error_dict[sketch][sketch_size].append(l2)

            elif sketch == CS:
                width = int(split_line[3])
                n_rows = int(split_line[5])
                l2 = float(split_line[9])

                if n_rows == fixed_cs:
                    if width not in error_dict[sketch]:
                        error_dict[sketch][width] = []
                    error_dict[sketch][width].append(l2)

    return speed_dict, error_dict

def get_median(speed_dict_all, error_dict_all):

    speed_dict = {}
    error_dict = {}

    for sketch in SKETCHES:
        speed_dict[sketch] = []
        error_dict[sketch] = []

        for size in speed_dict_all[sketch]:
            med = statistics.median(speed_dict_all[sketch][size])
            speed_dict[sketch].append((size, med))
        
        for size in error_dict_all[sketch]:
            med = statistics.median(error_dict_all[sketch][size])
            error_dict[sketch].append((size, med))

    return speed_dict, error_dict

def plot_speed_and_error(alpha:float, fixed_orcas:int, fixed_cs:int): # ORCAS array counter, CS row number

    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = 'alpha_' + alpha_str + '/'

    speed_data = {}
    error_data = {}

    for sketch in SKETCHES:
        speed_data[sketch] = []
        error_data[sketch] = []

    # e.g. alpha_1-0/seed_1_test_orcas_error_on_arrival.txt
    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()

        if ORCAS.lower() in fn:
            indiv_data = indiv_data[1:] # remove zipf generation line
            if 'speed' in fn:
                speed_data[ORCAS] = speed_data[ORCAS] + indiv_data
            elif 'error_on_arrival' in fn:
                error_data[ORCAS] = error_data[ORCAS] + indiv_data

        elif 'count_sketch' in fn:
            if 'speed' in fn:
                speed_data[CS] = speed_data[CS] + indiv_data
            elif 'error_on_arrival' in fn:
                error_data[CS] = error_data[CS] + indiv_data
    
    speed_dict_all, error_dict_all = get_all_values(speed_data, error_data, fixed_orcas, fixed_cs)
    speed_dict, error_dict = get_median(speed_dict_all, error_dict_all)

    fig, (speed_ax, error_ax) = plt.subplots(1, 2)

    for sketch in SKETCHES:
        
        if sketch == ORCAS:
            speed_x = [sketch_size*4/1024 for (sketch_size, speed) in speed_dict[sketch]]
            speed_y = [speed for (sketch_size, speed) in speed_dict[sketch]]
            speed_ax.plot(speed_x, speed_y, label=str(sketch))

            error_x = [sketch_size*4/1024 for (sketch_size, speed) in error_dict[sketch]]
            error_y = [speed for (sketch_size, speed) in error_dict[sketch]]
            error_ax.plot(error_x, error_y, label=str(sketch))

        elif sketch == CS:
            speed_x = [fixed_cs*width*4/1024 for (width, speed) in speed_dict[sketch]]
            speed_y = [speed for (width, speed) in speed_dict[sketch]]
            speed_ax.plot(speed_x, speed_y, label=str(sketch))

            error_x = [fixed_cs*width*4/1024 for (width, l2) in error_dict[sketch]]
            error_y = [l2 for (width, l2) in error_dict[sketch]]
            error_ax.plot(error_x, error_y, label=str(sketch))

    N = speed_data[ORCAS][0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | ORCAS array counter = ' + str(fixed_orcas)  + ' | CS row number = ' + str(fixed_cs)
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
    
    plot_speed_and_error(1.0, fixed_orcas=3, fixed_cs=3)
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()        
