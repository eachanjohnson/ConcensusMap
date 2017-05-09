#!/usr/bin/env python

import yaml
import argparse
import shutil
import pandas as pd
import csv
import re
import os
import getpass


class ConfDict(object):

    def __init__(self, options, basepath):
        # Store all the properties to the dict "datastore"
        object.__setattr__(self, 'datastore', {})
        self.datastore['basepath'] = basepath

        self.options = options
        self.config_file = options.config_file
        self.mydict = yaml.load(open(self.config_file))
        self.ldelim = "/"

    def __getattr__(self, key):
        if key in ['options', 'mydict']:
            return super(ConfDict, self).__getattr__(key)
        else:
            return self.datastore[key]

    def __setattr__(self, key, value):
        if key in ['options', 'mydict']:
            super(ConfDict, self).__setattr__(key, value)
        else:
            self.datastore[key] = value

    def storeConfigFromConfig(self):
        mydict = self.mydict

        self.i5_barcodes_file = mydict['i5_barcodes_file']
        self.i7_barcodes_file = mydict['i7_barcodes_file']
        self.sbc_barcodes_file = mydict['sbc_barcodes_file']
        self.p5_to_stagger_file = mydict['p5_to_stagger_file']
        self.UGER_cbp = mydict['UGER_cbp']
        self.i5_plates_file = mydict['i5_plates_file']
        self.i7_plates_file = mydict['i7_plates_file']
        self.strains_map_file = mydict['strains_map_file']


    def storeConfigFromOptions(self):
        options = self.options
        self.use_qsub = options.use_qsub
        self.project_id = options.project_id

    def storeConfigMixed(self):
        options = self.options
        mydict = self.mydict
        ldelim = self.ldelim

        suffix_short = ''
        if options.Suffix_short != 'none':
            suffix_short = options.Suffix_short
        else:
            suffix_short = mydict['Suffix_short']
        self.suffix_short = suffix_short

        suffix_long = ''
        if options.Suffix_long != 'none':
            suffix_long = options.Suffix_long
        else:
            suffix_long = mydict['Suffix_long']
        self.suffix_long = suffix_long

        raw_seq_path = ''
        if options.seq_id != 'none':
            Seq_base = mydict['Seq_base']
            raw_seq_path = Seq_base + self.ldelim + options.seq_id
        elif options.raw_seq_path != 'none':
            raw_seq_path = options.raw_seq_path
        else:
            raise argparse.ArgumentTypeError('Either seq_id or raw_seq_path need to be provided.')
        self.Input_dir = raw_seq_path

        self.login_name = getpass.getuser()
        temp_path = ''
        if options.temp_path != 'none':
            temp_path = options.temp_path
        else:
            temp_path = mydict['Temp_path']

        if options.use_login_name:
            self.Temp_dir = temp_path + ldelim + self.login_name + ldelim + self.project_id
        else:
            self.Temp_dir = temp_path + ldelim + self.project_id

        results_path = ''
        if options.results_path != 'none':
            results_path = options.results_path
        else:
            results_path = mydict['Results_path']

        if options.use_login_name:
            self.Results_path = results_path + ldelim + self.login_name + ldelim + self.project_id
        else:
            self.Results_path = results_path + ldelim + self.project_id


    def createSubPaths(self):
        Out_dir = self.Temp_dir
        ldelim = self.ldelim
        use_qsub = self.use_qsub

        UGER_cbp_dir = Out_dir + ldelim + "UGER_cbp"
        Log_dir = Out_dir + ldelim + "logdir"
        Count_dir = Out_dir + ldelim + "countdir"
        Summary_dir = Out_dir + ldelim + "summarydir"
        
        if use_qsub and os.path.exists(UGER_cbp_dir):
            shutil.rmtree(UGER_cbp_dir)
        if not os.path.exists(Out_dir):
            os.makedirs(Out_dir)
        if not os.path.exists(Log_dir):
            os.makedirs(Log_dir)
        if not os.path.exists(Count_dir):
            os.makedirs(Count_dir)
        if not os.path.exists(Summary_dir):
            os.makedirs(Summary_dir)

        self.UGER_cbp_dir = UGER_cbp_dir
        self.Log_dir = Log_dir
        self.Count_dir = Count_dir
        

    def loadConfig(self):
        self.storeConfigFromConfig()
        self.storeConfigFromOptions()
        self.storeDerivedPaths()
        self.storeConfigMixed()
        self.createSubPaths()

    def storeDerivedPaths(self):
        ldelim = self.ldelim
        Script_dir = self.basepath
        self.concensus_mono = Script_dir + ldelim + "concensus_mono"

    def dumpConfigLog(self):
        config_log_out = self.Log_dir + self.ldelim + "config_log_out.txt"
        with open(config_log_out, 'w') as outfile:
            yaml.dump(self.datastore, outfile, default_flow_style=False)
        return config_log_out

