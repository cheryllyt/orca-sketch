import os
import matplotlib.pyplot as plt
import statistics

ORCAS_NEW = 'ORCAS_NEW'
ORCAS_OLD = 'ORCAS_OLD'

SKETCHES = [ORCAS_NEW, ORCAS_OLD]

def get_all_values(speed_data, fixed_orcas):

    speed_dict = {}
    
    for sketch in SKETCHES:

        for array_counter in fixed_orcas:
            key = sketch + '-' + str(array_counter)

            speed_dict[key] = {}

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

    return speed_dict

def get_median(speed_dict_all):

    speed_dict = {}

    for key in speed_dict_all:
        speed_dict[key] = []

        for size in speed_dict_all[key]:
            med = statistics.median(speed_dict_all[key][size])
            speed_dict[key].append((size, med))

    return speed_dict

def plot_speed_and_error(alpha:float, fixed_orcas:int): # ORCAS array counters, CMS row numbers

    speed_data = {}

    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]

    # with template parameters
    alpha_folder_name = '../alpha_' + alpha_str + '/'
 
    speed_data[ORCAS_NEW] = []

    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()
        indiv_data = indiv_data[1:] # remove zipf generation line
        if 'speed' in fn:
                speed_data[ORCAS_NEW] = speed_data[ORCAS_NEW] + indiv_data

    # without template parameters
    alpha_folder_name = '../alpha_' + alpha_str + '_old_code/'

    speed_data[ORCAS_OLD] = []

    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()
        indiv_data = indiv_data[1:] # remove zipf generation line
        if 'speed' in fn:
                speed_data[ORCAS_OLD] = speed_data[ORCAS_OLD] + indiv_data
        
    speed_dict_all = get_all_values(speed_data, fixed_orcas)
    speed_dict = get_median(speed_dict_all)

    fig, (speed_ax) = plt.subplots(1, 1)

    for key in speed_dict:
        
        if ORCAS_NEW in key:
            key_label = 'w/: ' + key.split('-')[1]

        elif ORCAS_OLD in key:
            key_label = 'w/o: ' + key.split('-')[1]

        speed_x = [sketch_size*4/1024 for (sketch_size, speed) in speed_dict[key]]
        speed_y = [speed for (sketch_size, speed) in speed_dict[key]]
        speed_ax.plot(speed_x, speed_y, label=str(key_label))

    N = speed_data[ORCAS_NEW][0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | ORCAS config: array counter | template parameters'
    # fig.suptitle(fig_title, fontsize=18)

    speed_ax.set_xlabel('Memory [KB]', fontsize=22)
    speed_ax.set_ylabel('Throughput [Mops]', fontsize=22)
    speed_ax.tick_params(axis='both', which='both', labelsize=20, width=1, length=4)
    speed_ax.set_xscale('log',base=10)
    speed_ax.legend(loc="upper right", prop={"size":20}, ncol=len([key for key in speed_dict]), bbox_to_anchor=(1.12, 1.14))

def plot_all_alpha():
    
    plot_speed_and_error(1.0, fixed_orcas=[2,3,4])
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()        
