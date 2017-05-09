#!/usr/bin/env python

import yaml
import argparse
import shutil
import pandas as pd
import csv
import re
import os
import ntpath
import collections
from subprocess import call

from confdict import ConfDict

class Concensus:
    def __init__(self, options):
        """ """
        self.options = options
        basepath = os.path.dirname(os.path.realpath(__file__))
        confd = ConfDict(options, basepath)
        confd.loadConfig()
        self.config_log_out = confd.dumpConfigLog()
        self.confd = confd



    def get_prefix_set_paired(self):
        confd = self.confd
        Input_dir = confd.Input_dir
        suffix_short = confd.suffix_short
        suffix_long = confd.suffix_long
   
        prefix_set = set()
        for file in os.listdir(Input_dir):
            filename = os.path.basename(os.path.normpath(file))
            lsuffix_short = suffix_short
            lsuffix_long = suffix_long
            if filename.endswith("gz"):
                lsuffix_short += ".gz"
                lsuffix_long += ".gz"
            lprefix = ''
            if filename.endswith(lsuffix_short):
                lprefix = filename.replace(lsuffix_short, '')
            elif filename.endswith(lsuffix_long):
                lprefix = filename.replace(lsuffix_long, '')

            if lprefix and lprefix not in prefix_set:
                prefix_set.add(lprefix)
        return prefix_set

    def get_fastq_file(self, lprefix, suffix):
        confd = self.confd
        Input_dir = confd.Input_dir
        ldelim = confd.ldelim
        lfile = Input_dir + ldelim + lprefix + suffix
        if not os.path.isfile(lfile):
            lfile += ".gz"
            if not os.path.isfile(lfile):
                raise OSError("File not found: " + lfile1)
        return lfile

    def build_core_jobs(self, prefix_set):
        confd = self.confd
        suffix_short = confd.suffix_short
        suffix_long = confd.suffix_long
        concensus_mono = confd.concensus_mono
        i5_barcodes_file = confd.i5_barcodes_file
        i7_barcodes_file = confd.i7_barcodes_file
        sbc_barcodes_file = confd.sbc_barcodes_file
        p5_to_stagger_file = confd.p5_to_stagger_file
        Count_dir = confd.Count_dir
        Log_dir = confd.Log_dir
        UGER_cbp = confd.UGER_cbp
        UGER_cbp_dir = confd.UGER_cbp_dir
        ldelim = confd.ldelim
        use_qsub = confd.use_qsub
        
        concensus_job_path = Log_dir + ldelim + "concensus_joblist.txt"
        jfile = open(concensus_job_path, "w")


        for lprefix in prefix_set:
            file_short = self.get_fastq_file(lprefix, suffix_short) 
            file_long = self.get_fastq_file(lprefix, suffix_long)
            print("lprefix: " + lprefix + " file_short: " + file_short + " file_long: " + file_long )
 
            cmd_str = concensus_mono + " --i5 " + i5_barcodes_file + " --i7 " + i7_barcodes_file + " --sbc " + sbc_barcodes_file + " --stagger " + p5_to_stagger_file +  " --short_read " + file_short + " --long_read " + file_long + " -p " + lprefix + " -o " + Count_dir
            loutfile = Log_dir + ldelim + lprefix + "_out.txt"
            lerrfile = Log_dir + ldelim + lprefix + "_err.txt"
            cmd_str2 = cmd_str + " 1> " + loutfile + " 2> " + lerrfile + "\n"
            print(cmd_str2)
            jfile.write(cmd_str2)
        jfile.close()
        lmemory = "8"
        joblist_cmd = UGER_cbp + " --cmds_file " + concensus_job_path + \
                                " --batch_size 1" + \
                                " --queue long" + \
                                " --num_cores 1" + \
                                " --memory " + lmemory + \
                                " --tracking_dir " + UGER_cbp_dir + \
                                " --project_name broad --bash_header /broad/IDP-Dx_work/nirmalya/bash_header"
        if use_qsub:
            call(joblist_cmd.split())
 

    def map_lane(self, lane, count_file):
        print("lane: " + lane + " count_file: " + count_file)
        # 

      
    def combine_lanes(self, prefix_set):
        """ Get the names of the files in the specific orders """
        confd = self.confd
        Count_dir = confd.Count_dir
        ldelim =  confd.ldelim

        prefix_dict = dict()
        reg_str = "_\\S+"
        for lprefix in prefix_set:
            lseq = re.sub(reg_str, '', lprefix)
            prefix_dict[lseq] = lprefix
        od = collections.OrderedDict(sorted(prefix_dict.items()))
        
        for lseq, lprefix in od.iteritems():
            # Get the complete file
            count_file = Count_dir + ldelim + lprefix + "_outfile.txt"
            self.map_lane(lseq, count_file)
            
            
        
    
  
    def mainFunc(self):
        """ """
        prefix_set = self.get_prefix_set_paired()
        self.build_core_jobs(prefix_set)
        self.combine_lanes(prefix_set)
            
               


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process the options.')
    parser.add_argument('--config_file', '-c', type = str, required = True, help ='Path to main config file')
    parser.add_argument('--project_id', required = True, default = 'none', help = 'Project id')
    parser.add_argument('--seq_id', required = False, default = 'none', help = 'Sequencing id used to make raw_seq_path')
    parser.add_argument('--no_qsub', dest = 'use_qsub', action = 'store_false', default = True, help = 'Does not submit qsub jobs.' )
    parser.add_argument('--raw_seq_path', required = False, default = 'none', help = 'Directory for raw sequene files (absolute)')
    parser.add_argument('--temp_path', required = False, default = 'none', help = 'Will contain the temporary results')
    parser.add_argument('--results_path', required = False, default = 'none', help = 'Will contain the path to the results')
    parser.add_argument('--Suffix_short', default = 'none', required = False, help = 'Update the value of Suffix_short')
    parser.add_argument('--Suffix_long', default = 'none', required = False, help = 'Update the value of Suffix_long')
    parser.add_argument("--no_login_name", dest = "use_login_name", action = 'store_false', default = True, help = 'Generate results in a username specific directory')

    options = parser.parse_args()
    concen = Concensus(options)
    concen.mainFunc()
    
    
