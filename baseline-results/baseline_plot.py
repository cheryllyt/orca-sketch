import os
import matplotlib.pyplot as plt
import statistics

class BaselinePlot:

    def read_data(self, alpha:float, folder_path:str):

        # alpha must be 3 char (1 digit before '.' and 1 digit after)
        alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
        folder_name = folder_path + '/alpha_' + alpha_str + '/'

        speed_data = []
        error_data = []
        hh_data = []

        # e.g. alpha_1-0/test_baseline_cms_error_on_arrival_seed_1.txt
        for fn in os.listdir(folder_name):
            indiv_data = open(folder_name + fn).readlines()
            if 'speed' in fn:
                speed_data = speed_data + indiv_data
            elif 'error_on_arrival' in fn:
                error_data = error_data + indiv_data
            elif 'final_error' in fn:
                hh_data = hh_data + indiv_data
        
        return speed_data, error_data, hh_data

    def get_all_values(self, speed_data=[], error_data=[], hh_data=[], row_numbers=[], thresholds=[]):

        speed_dict = {}
        error_dict = {}
        hh_dict = {}

        for r in row_numbers:
            speed_dict[r] = {}
            error_dict[r] = {}
            hh_dict[r] = {}

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

        for line in hh_data:
            split_line = line.split('\t')
            w = int(split_line[3])
            r = int(split_line[5])
            rel_error = []
            j = 9
            for threshold in thresholds:
                rel_error.append(float(split_line[j]))
                j += 4

            if r in hh_dict:
                if w not in hh_dict[r]:
                    hh_dict[r][w] = []
                hh_dict[r][w].append(rel_error)

        return speed_dict, error_dict, hh_dict

    def get_median(self, speed_dict_all=[], error_dict_all=[], hh_dict_all=[], row_numbers=[], thresholds=[]):

        speed_dict = {}
        error_dict = {}
        hh_dict = {}

        for r in row_numbers:
            speed_dict[r] = []
            error_dict[r] = []
            hh_dict[r] = []

        for r in row_numbers:
            for w in speed_dict_all[r]:
                med = statistics.median(speed_dict_all[r][w])
                speed_dict[r].append((w, med))
            
            for w in error_dict_all[r]:
                med = statistics.median(error_dict_all[r][w])
                error_dict[r].append((w, med))

            for w in hh_dict_all[r]:
                meds = []
                n_seeds = len(hh_dict_all[r][w])
                for i in range(len(thresholds)):
                    to_compare = []
                    for j in range(n_seeds):
                        to_compare.append(hh_dict_all[r][w][j][i])
                    med = statistics.median(to_compare)
                    meds.append(med)
                hh_dict[r].append((w, meds))

        return speed_dict, error_dict, hh_dict
    
    def draw_subplot(self, plot_speed_error_bool:bool, plot_hh_bool:bool, alpha:float, folder_path:str, row_numbers=[], thresholds=[], fixed_sketch_size=None):

        speed_data, error_data, hh_data = self.read_data(alpha, folder_path)
        speed_dict_all, error_dict_all, hh_dict_all = self.get_all_values(speed_data, error_data, hh_data, row_numbers, thresholds)
        speed_dict, error_dict, hh_dict = self.get_median(speed_dict_all, error_dict_all, hh_dict_all, row_numbers, thresholds)

        if plot_speed_error_bool:

            fig, (speed_ax, error_ax) = plt.subplots(1, 2)

            for r in row_numbers:
                speed_x = [r*w*4/1024 for (w, speed) in speed_dict[r]]
                speed_y = [speed for (w, speed) in speed_dict[r]]
                speed_ax.plot(speed_x, speed_y, label=str(r))

                error_x = [r*w*4/1024 for (w, l2) in error_dict[r]]
                error_y = [l2 for (w, l2) in error_dict[r]]
                error_ax.plot(error_x, error_y, label=str(r))

            N = speed_data[0].split('\t')[1]
            fig_title = 'N = ' + N + ' | alpha = ' + str(alpha)
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
            error_ax.legend(loc="upper right", prop={"size":20}, ncol=len([key for key in speed_dict]), bbox_to_anchor=(0.7, 1.14))
        
        if plot_hh_bool:

            fig, (hh_ax) = plt.subplots(1, 1)

            for r in row_numbers:
                hh_x = [threshold for threshold in thresholds]
                hh_y = []
                for (width, rel_error) in hh_dict[r]:
                    if (r*width) == fixed_sketch_size:
                        hh_y = rel_error
                        break
                hh_ax.plot(hh_x, hh_y, label=str(r)+' rows')

            N = speed_data[0].split('\t')[1]
            fig_title = 'N = ' + N + ' | alpha = ' + str(alpha)
            if fixed_sketch_size is not None:
                fig_title = fig_title + ' | fixed sketch size = ' + str(fixed_sketch_size)
            fig.suptitle(fig_title, fontsize=18)

            hh_ax.set_xlabel('Threshold', fontsize=18)
            hh_ax.set_ylabel('ARE', fontsize=18)
            hh_ax.tick_params(axis='both', which='both', labelsize=15)
            hh_ax.set_xscale('log',base=10)
            hh_ax.set_yscale('log',base=10)
            hh_ax.legend(loc="upper right", prop={"size":11})

    def show_plots(self):
        plt.show()
