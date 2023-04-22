ratio_list = [
3,
5,
4,
5,
6,
6,
6,
5,
6,
9,
10,
13,
10,
5,
4,
3,
]
latency_start = 10000000
total_erase_latency = 17500000
# latency_sequence = [1000000, 2500000, 3500000, 3500000, 3500000, 3500000,
#                     3500000, 3500000, 3500000]
latency_sequence = [3500000, 3500000, 3500000, 3500000, 3500000,
                    3500000, 3500000, 3500000]

max_erase_latency = []
acc_lat = 0
for lat in latency_sequence:
    if acc_lat < total_erase_latency:
        acc_lat += lat
        max_erase_latency.append(lat)

block_index_file = "/root/work_dir/projects/MQSim/block_index.txt"
category = {}
for idx, ratio in enumerate(ratio_list):
    category[idx] = {
        'ratio' : ratio,
        'latency' : latency_start
    }
    latency_start += 500000

data = {
    'max_erase_latency' : max_erase_latency,
    'block_index_file' : block_index_file,
    'category' : category,
}
import yaml
with open('base_45k.yaml', 'w') as fout:
    yaml.dump(data, fout, default_flow_style=False)
