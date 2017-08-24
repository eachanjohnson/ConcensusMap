file1 = '/broad/IDP-Dx_storage/Concensus_results/SN0075811_summary.csv'
file2 = '/broad/IDP-Dx_storage/Concensus_results/SN0075812_summary.csv'
outfile = '/broad/IDP-Dx_storage/Concensus_results/SN0118837AB_new_py.csv'
count = 0

def add_str(var1, var2):
   val1 = int(var1)
   val2 = int(var2)
   total = val1 + val2
   total_str = str(total)
   return total_str;

with open(file1) as f1, open(file2) as f2, open(outfile, "w") as of:
    header1 = f1.next()
    f2.next()
    header = header1.strip()
    of.write(header + "\n")
    for line1_t in f1:
        line2_t = f2.next();
        line1 = line1_t.strip()
        line2 = line2_t.strip()
        #print("line1: " + line1)
        #print("line2: " + line2)
        parts1 = line1.split(',')
        parts2 = line2.split(',')
        unique_1 = parts1[-3]
        unique_2 = parts2[-3]
        unique_total = add_str(unique_1, unique_2)
        mixed_1 = parts1[-2]
        mixed_2 = parts2[-2]
        mixed_total = add_str(mixed_1, mixed_2)
        all_1 = parts1[-1]
        all_2 = parts2[-1]
        all_total = add_str(all_1, all_2)
        temp = parts1[:-3 or None]
        total_lst = temp + [unique_total, mixed_total, all_total]
        total_str = ','.join(total_lst)
        of.write(total_str + "\n")
        count = count + 1
        #if count == 10000:
        #    break
