ratio_list = [
2,
23,
28,
25,
13,
5,
2,
2,
]
latency_start = 2000000
total_erase_latency = 5500000
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
with open('aero2_15k.yaml', 'w') as fout:
    yaml.dump(data, fout, default_flow_style=False)
