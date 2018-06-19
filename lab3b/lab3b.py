#!/usr/local/cs/bin/python

#NAME: Shawye Ho,Yanyin Liu


from collections import defaultdict
import sys
import os.path

if len(sys.argv) != 2:
    print >> sys.stderr, 'Invalid Argument/Parameter. Usage: ./lab3b [file image .csv]'
    sys.exit(1)



argtemp = sys.argv[1]
argtemp = argtemp[-4:] 
if argtemp != '.csv':
    print >> sys.stderr, 'Please use an appropriate .csv file'
    sys.exit(1)

if not os.path.isfile(sys.argv[1]):
    print >> sys.stderr, 'Invalid file'
    sys.exit(1)

err = 0

with open(sys.argv[1]) as cfd:
    for line in cfd:
       
        str_list = line.split(',') 
        type_ = str_list[0] 
        if type_ == 'SUPERBLOCK':
            str_first_non_reserved_inode = str_list[7][:-1] 
            str_total_blocks = str_list[1]
        elif type_ == 'GROUP':
            str_total_inodes_in_group = str_list[3]
            first_inode_block = str_list[8][:-1] 
            continue 

total_blocks = int(str_total_blocks)
first_non_reserved_inode = int(str_first_non_reserved_inode)
first_data_block = int(float(str_total_inodes_in_group)/8 + float(first_inode_block)); 
total_inodes_in_group = int(str_total_inodes_in_group)

block_dictionary = defaultdict(list)

inode_dictionary = defaultdict(list)

ref_dictionary = dict()


parent_dictionary = dict()

with open(sys.argv[1]) as cfd:
    for line in cfd:
        str_list = line.split(',') 
        type_ = str_list[0] 
        if type_ == 'BFREE':
            block_dictionary[str_list[1][:-1]].insert(0, 'free') 
        elif type_ == 'INODE':
           
            if str_list[2] == 'f' or str_list[2] == 'd': 
                for num in range(0, 15): 
                    if num == 14:
                        temp2 = int(str_list[num + 12][:-1]) 
                    else:
                        temp2 = int(str_list[num + 12])
                   
                    if temp2 == 0:
                        continue
                    if num == 13:
                        off = (12 + 256)
                        block_type = 'DOUBLE INDIRECT'
                        level = '2'
                    elif num == 14:
                        off = (12 + 256 + (256*256))
                        block_type = 'TRIPLE INDIRECT'
                        level = '3'
                    elif num == 12:
                        off = num
                        block_type = 'INDIRECT'
                        level = '1'
                    else:
                        off = num
                        block_type = ''
                        level = '0'
                    if temp2 > total_blocks:
                        if block_type == '':
                            print 'INVALID BLOCK', str(temp2), 'IN INODE', str_list[1], 'AT OFFSET', off
                        else:
                            print 'INVALID', block_type, 'BLOCK', str(temp2), 'IN INODE', str_list[1], 'AT OFFSET', off
                        err = 1
                    elif temp2 < first_data_block: #less than first
                        if block_type == '':
                            print 'RESERVED BLOCK', str(temp2), 'IN INODE', str_list[1], 'AT OFFSET', off
                        else:
                            print 'RESERVED', block_type, 'BLOCK', str(temp2), 'IN INODE', str_list[1], 'AT OFFSET', off
                        err = 1
                    else:
                        block_dictionary[str(temp2)].append([level, str_list[1], off])
           
            inode_dictionary[str_list[1]].append('allocated') 
            inode_dictionary[str_list[1]].append(str_list[6]) 
        elif type_ == 'IFREE':
            inode_dictionary[str_list[1][:-1]].insert(0, 'free')
        elif type_ == 'INDIRECT':
            temp = int(str_list[5][:-1]) 
            if temp > total_blocks:
                err = 1
                if str_list[2] == '1':
                    print 'INVALID INDIRECT BLOCK', temp, 'IN INODE', str_list[1], 'AT OFFSET', str_list[3]
                if str_list[2] == '2':
                    print 'INVALID DOUBLE INDIRECT BLOCK', temp, 'IN INODE', str_list[1], 'AT OFFSET', str_list[3]
                if str_list[2] == '3':
                    print 'INVALID TRIPLE INDIRECT BLOCK', temp, 'IN INODE', str_list[1], 'AT OFFSET', str_list[3]
            elif temp < first_data_block:
                err = 1
                if str_list[2] == '1':
                    print 'RESERVED INDIRECT BLOCK', temp, 'IN INODE', str_list[1], 'AT OFFSET', str_list[3]
                if str_list[2] == '2':
                    print 'RESERVED DOUBLE INDIRECT BLOCK', temp, 'IN INODE', str_list[1], 'AT OFFSET', str_list[3]
                if str_list[2] == '3':
                    print 'RESERVED TRIPLE INDIRECT BLOCK', temp, 'IN INODE', str_list[1], 'AT OFFSET', str_list[3]
            else:
                block_dictionary[str_list[5][:-1]].append([str_list[2], str_list[1], str_list[3]])
        else:
            continue 
with open(sys.argv[1]) as cfd:
    for line in cfd:
        str_list = line.split(',') 
        type_ = str_list[0] 
        if type_ == 'DIRENT':
            inode_dictionary[str_list[1]].append([str_list[6][:-1], str_list[3]]) 
            if str_list[6][:-1] != '\'.\'' and str_list[6][:-1] != '\'..\'' and str_list[3] != str_list[1]: 
                parent_dictionary[str_list[3]] = str_list[1]
            if str_list[3] in ref_dictionary: 
                ref_dictionary[str_list[3]] += 1
            else:
                ref_dictionary[str_list[3]] = 1
        else:
            continue 


for bnum in range(first_data_block, total_blocks):
    keys = block_dictionary.keys()
    str_bnum = str(bnum)
   
    if not str_bnum in keys:
        print 'UNREFERENCED BLOCK', bnum
        err = 1
    if str_bnum in keys:
        if block_dictionary[str_bnum][0] == 'free' and len(block_dictionary[str_bnum]) > 1:
            print 'ALLOCATED BLOCK', bnum, 'ON FREELIST'
            err = 1
        
        if (block_dictionary[str_bnum][0] != 'free' and len(block_dictionary[str_bnum])>1) or (len(block_dictionary[str_bnum]) > 2): 
                    continue
                err = 1
                if elem[0] == '1':
                    print 'DUPLICATE INDIRECT BLOCK', bnum, 'IN INODE', elem[1], 'AT OFFSET', elem[2]
                elif elem[0] == '2':
                    print 'DUPLICATE DOUBLE INDIRECT BLOCK', bnum, 'IN INODE', elem[1], 'AT OFFSET', elem[2]
                elif elem[0] == '3':
                    print 'DUPLICATE TRIPLE INDIRECT BLOCK', bnum, 'IN INODE', elem[1], 'AT OFFSET', elem[2]
                else:
                    print 'DUPLICATE BLOCK', bnum, 'IN INODE', elem[1], 'AT OFFSET', elem[2]

for inum in range(first_non_reserved_inode, total_inodes_in_group):
    keys = inode_dictionary.keys()
    str_inum = str(inum)
    ref_keys = ref_dictionary.keys()
  
    if str_inum in keys:
        if inode_dictionary[str_inum][0] == 'free' and len(inode_dictionary[str_inum]) > 1:
            print 'ALLOCATED INODE', inum, 'ON FREELIST'
            err = 1
        if inode_dictionary[str_inum][0] == 'free' and len(inode_dictionary[str_inum]) > 1: 
            if str_inum in ref_keys:
                if inode_dictionary[str_inum][2] != str(ref_dictionary[str_inum]): 
                   
                    print 'INODE', inum, 'HAS', ref_dictionary[str_inum], 'LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][2]
                    err = 1
            else:
                if inode_dictionary[str_inum][2] != '0': 
                    print 'INODE', inum, 'HAS 0 LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][2]
                    err = 1
        if inode_dictionary[str_inum][0] != 'free': 
            if str_inum in ref_keys:
                if inode_dictionary[str_inum][1] != str(ref_dictionary[str_inum]):
                    print 'INODE', inum, 'HAS', ref_dictionary[str_inum], 'LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][1]
                    err = 1
            else:
                if inode_dictionary[str_inum][1] != '0': 
                    print 'INODE', inum, 'HAS 0 LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][1]
                    err = 1
            
        if inode_dictionary[str_inum][0] == 'free' and len(inode_dictionary[str_inum]) >= 4: 
            for j in range (3, len(inode_dictionary[str_inum])):
                if int(inode_dictionary[str_inum][j][1]) < 1 or int(inode_dictionary[str_inum][j][1]) > total_inodes_in_group: 
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'INVALID INODE', inode_dictionary[str_inum][j][1]
                    err = 1
                    continue
                if inode_dictionary[str_inum][j][1] in keys:
                    temp_list = list(inode_dictionary[inode_dictionary[str_inum][j][1]])
                    if len(temp_list) == 1: 
                        print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][j][1]
                        err = 1
                    
                else: 
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][j][1]
                    err = 1
                if inode_dictionary[str_inum][j][0] == '\'.\'':
                    if int(inode_dictionary[str_inum][j][1]) != inum:
                        print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'LINK TO INODE', inode_dictionary[str_inum][j][1], 'SHOULD BE', inum
                        err = 1
                if inode_dictionary[str_inum][j][0] == '\'..\'':
                    if int(inode_dictionary[str_inum][j][1]) != int(parent_dictionary[str_inum]):
                        print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'LINK TO INODE', inode_dictionary[str_inum][j][1], 'SHOULD BE', parent_dictionary[str_inum]
                        err = 1
        if inode_dictionary[str_inum][0] == 'allocated' and len(inode_dictionary[str_inum]) >= 3: 
            for k in range (2, len(inode_dictionary[str_inum])):
                if int(inode_dictionary[str_inum][k][1]) < 1 or int(inode_dictionary[str_inum][k][1]) > total_inodes_in_group: 
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'INVALID INODE', inode_dictionary[str_inum][k][1]
                    err = 1
                    continue
                if inode_dictionary[str_inum][k][1] in keys:
                    temp_list = list(inode_dictionary[inode_dictionary[str_inum][k][1]])
                    if len(temp_list) == 1:
                        print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][k][1]
                        err = 1
                   
                else:
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][k][1]
                    err = 1
                if inode_dictionary[str_inum][k][0] == '\'.\'':
                    if int(inode_dictionary[str_inum][k][1]) != inum:
                        print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'LINK TO INODE', inode_dictionary[str_inum][k][1], 'SHOULD BE', inum
                        err = 1
                if inode_dictionary[str_inum][k][0] == '\'..\'':
                    if int(inode_dictionary[str_inum][k][1]) != int(parent_dictionary[str_inum]):
                        print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'LINK TO INODE', inode_dictionary[str_inum][k][1], 'SHOULD BE', parent_dictionary[str_inum]
                        err = 1
    else: 
        print 'UNALLOCATED INODE', inum, 'NOT ON FREELIST'
        err = 1
       

inum = 2 
keys = inode_dictionary.keys()
str_inum = str(inum)
ref_keys = ref_dictionary.keys()

if str_inum in keys:
    if inode_dictionary[str_inum][0] == 'free' and len(inode_dictionary[str_inum]) > 1:
        print 'ALLOCATED INODE', inum, 'ON FREELIST'
        err = 1
    if inode_dictionary[str_inum][0] == 'free' and len(inode_dictionary[str_inum]) > 1: 
        if str_inum in ref_keys:
            if inode_dictionary[str_inum][2] != str(ref_dictionary[str_inum]): 
               
                print 'INODE', inum, 'HAS', ref_dictionary[str_inum], 'LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][2]
                err = 1
        else:
            if inode_dictionary[str_inum][2] != '0': 
              
                print 'INODE', inum, 'HAS 0 LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][2]
                err = 1
    if inode_dictionary[str_inum][0] != 'free': 
        if str_inum in ref_keys:
            if inode_dictionary[str_inum][1] != str(ref_dictionary[str_inum]): 
                
                print 'INODE', inum, 'HAS', ref_dictionary[str_inum], 'LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][1]
                err = 1
        else:
            if inode_dictionary[str_inum][1] != '0': 
               
                print 'INODE', inum, 'HAS 0 LINKS BUT LINKCOUNT IS', inode_dictionary[str_inum][1]
                err = 1
           
    if inode_dictionary[str_inum][0] == 'free' and len(inode_dictionary[str_inum]) >= 4: 
        for j in range (3, len(inode_dictionary[str_inum])):
            if int(inode_dictionary[str_inum][j][1]) < 1 or int(inode_dictionary[str_inum][j][1]) > total_inodes_in_group: 
                print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'INVALID INODE', inode_dictionary[str_inum][j][1]
                err = 1
                continue
            if inode_dictionary[str_inum][j][1] in keys:
                temp_list = list(inode_dictionary[inode_dictionary[str_inum][j][1]])
                if len(temp_list) == 1: 
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][j][1]
                    err = 1
                  
            else: 
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][j][1]
                    err = 1
            if inode_dictionary[str_inum][j][0] == '\'.\'':
                if int(inode_dictionary[str_inum][j][1]) != inum:
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'LINK TO INODE', inode_dictionary[str_inum][j][1], 'SHOULD BE', inum
                    err = 1
            if inode_dictionary[str_inum][j][0] == '\'..\'': 
                if int(inode_dictionary[str_inum][j][1]) != inum:
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][j][0], 'LINK TO INODE', inode_dictionary[str_inum][j][1], 'SHOULD BE', inum
                    err = 1
    if inode_dictionary[str_inum][0] == 'allocated' and len(inode_dictionary[str_inum]) >= 3: 
        for k in range (2, len(inode_dictionary[str_inum])):
            if int(inode_dictionary[str_inum][k][1]) < 1 or int(inode_dictionary[str_inum][k][1]) > total_inodes_in_group: 
                print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'INVALID INODE', inode_dictionary[str_inum][k][1]
                err = 1
                continue
            if inode_dictionary[str_inum][k][1] in keys:
                temp_list = list(inode_dictionary[inode_dictionary[str_inum][k][1]])
                if len(temp_list) == 1: #only 'free'
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][k][1]
                    err = 1
                    
            else: 
                print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'UNALLOCATED INODE', inode_dictionary[str_inum][k][1]
                err = 1
            if inode_dictionary[str_inum][k][0] == '\'.\'':
                if int(inode_dictionary[str_inum][k][1]) != inum:
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'LINK TO INODE', inode_dictionary[str_inum][k][1], 'SHOULD BE', inum
                    err = 1
            if inode_dictionary[str_inum][k][0] == '\'..\'': 
                if int(inode_dictionary[str_inum][k][1]) != inum:
                    print 'DIRECTORY INODE', inum, 'NAME', inode_dictionary[str_inum][k][0], 'LINK TO INODE', inode_dictionary[str_inum][k][1], 'SHOULD BE', inum
                    err = 1
else: 
    print 'UNALLOCATED INODE', inum, 'NOT ON FREELIST'
    err = 1

if err == 1:
    sys.exit(2)
else:
    sys.exit(0)
