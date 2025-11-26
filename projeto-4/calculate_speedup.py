import re
from collections import defaultdict

def parse_time_file(filepath):
    data = defaultdict(list)
    
    with open(filepath, 'r') as f:
        for line in f:
            if not line.strip():
                continue
                
            # Regex to extract relevant fields
            # Example line: Tipo: Sequencial | Processos: 1 | Tempo: 0.376883033 s | Tamanho Matriz: 1024 | Iterações: 16
            match = re.search(r'Tipo: (\w+) \| Processos: (\d+) \| Tempo: ([\d\.]+) s', line)
            if match:
                type_ = match.group(1)
                procs = int(match.group(2))
                time_ = float(match.group(3))
                
                key = (type_, procs)
                data[key].append(time_)
    
    return data

def calculate_metrics(data):
    results = {}
    baseline_time = None
    
    # Find baseline (Sequencial, 1 process)
    if ('Sequencial', 1) in data:
        baseline_times = data[('Sequencial', 1)]
        baseline_time = sum(baseline_times) / len(baseline_times)
        results[('Sequencial', 1)] = {
            'avg_time': baseline_time,
            'speedup': 1.0
        }
    
    for key, times in data.items():
        if key == ('Sequencial', 1):
            continue
            
        avg_time = sum(times) / len(times)
        speedup = 0.0
        if baseline_time:
            speedup = baseline_time / avg_time
            
        results[key] = {
            'avg_time': avg_time,
            'speedup': speedup
        }
        
    return results

def print_table(results):
    print(f"{'Tipo':<12} | {'Procs':<5} | {'Média (s)':<10} | {'Speedup':<8}")
    print("-" * 45)
    
    # Sort keys: Sequencial first, then Paralelo by num procs
    sorted_keys = sorted(results.keys(), key=lambda k: (k[0] == 'Paralelo', k[1]))
    
    for key in sorted_keys:
        type_, procs = key
        metrics = results[key]
        print(f"{type_:<12} | {procs:<5} | {metrics['avg_time']:.6f}   | {metrics['speedup']:.2f}x")

if __name__ == "__main__":
    filepath = "assets/output/time_measure.txt"
    try:
        data = parse_time_file(filepath)
        if not data:
            print("Nenhum dado encontrado no arquivo.")
        else:
            metrics = calculate_metrics(data)
            print_table(metrics)
    except FileNotFoundError:
        print(f"Arquivo não encontrado: {filepath}")
