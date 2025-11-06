#!/usr/bin/env python3
"""
Graph Generator for Oven Cure Cycles
Reads CSV files and generates temperature vs time graphs
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from pathlib import Path
import sys
from datetime import datetime

def generate_graph(csv_path, output_dir=None):
    """
    Generate a graph from cure cycle CSV data
    
    Args:
        csv_path: Path to the CSV file
        output_dir: Optional output directory (defaults to same as CSV)
    """
    # Read the data
    try:
        df = pd.read_csv(csv_path)
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return False
    
    # Set up the figure with high quality
    plt.figure(figsize=(14, 8), dpi=150)
    
    # Plot each sensor
    plt.plot(df['Time(s)'], df['CH1_Air(°C)'], 
             label='CH1 - Air Temp', linewidth=2, color='#E74C3C', alpha=0.9)
    plt.plot(df['Time(s)'], df['CH2(°C)'], 
             label='CH2', linewidth=2, color='#3498DB', alpha=0.8)
    plt.plot(df['Time(s)'], df['CH3(°C)'], 
             label='CH3', linewidth=2, color='#2ECC71', alpha=0.8)
    plt.plot(df['Time(s)'], df['CH5(°C)'], 
             label='CH5', linewidth=2, color='#9B59B6', alpha=0.8)
    plt.plot(df['Time(s)'], df['CH6_IR(°C)'], 
             label='CH6 - IR Sensor (Part)', linewidth=2.5, color='#F39C12', alpha=0.9)
    
    # Plot setpoint as horizontal line
    setpoint = df['Setpoint(°C)'].iloc[0]
    plt.axhline(y=setpoint, color='#34495E', linestyle='--', 
                linewidth=2, label=f'Setpoint ({setpoint}°C)', alpha=0.7)
    
    # Add tolerance band (±10°C)
    plt.axhline(y=setpoint + 10, color='#7F8C8D', linestyle=':', 
                linewidth=1, alpha=0.5, label='Tolerance (±10°C)')
    plt.axhline(y=setpoint - 10, color='#7F8C8D', linestyle=':', 
                linewidth=1, alpha=0.5)
    
    # Detect and annotate part insertion (IR drop)
    ir_temps = df['CH6_IR(°C)'].values
    time_vals = df['Time(s)'].values
    
    # Find significant drops in IR temp (part insertion)
    for i in range(1, len(ir_temps) - 20):
        if ir_temps[i-1] - ir_temps[i] > 30:  # 30°C drop
            plt.axvline(x=time_vals[i], color='green', linestyle='--', 
                       linewidth=1.5, alpha=0.6)
            plt.annotate('Part Inserted', 
                        xy=(time_vals[i], ir_temps[i]), 
                        xytext=(time_vals[i] + 30, ir_temps[i] - 20),
                        arrowprops=dict(arrowstyle='->', color='green', lw=1.5),
                        fontsize=11, color='green', fontweight='bold')
            break
    
    # Formatting
    plt.xlabel('Time (seconds)', fontsize=14, fontweight='bold')
    plt.ylabel('Temperature (°C)', fontsize=14, fontweight='bold')
    plt.title('Powder Coating Oven - Cure Cycle Temperature Profile', 
              fontsize=16, fontweight='bold', pad=20)
    plt.legend(loc='best', fontsize=11, framealpha=0.95)
    plt.grid(True, alpha=0.3, linestyle='--')
    
    # Add info box
    total_time = df['Time(s)'].iloc[-1]
    max_temp = df[['CH1_Air(°C)', 'CH2(°C)', 'CH3(°C)', 'CH5(°C)', 'CH6_IR(°C)']].max().max()
    
    info_text = f'Cycle Duration: {total_time:.0f}s ({total_time/60:.1f} min)\n'
    info_text += f'Setpoint: {setpoint}°C\n'
    info_text += f'Max Temperature: {max_temp:.1f}°C'
    
    plt.text(0.02, 0.98, info_text,
             transform=plt.gca().transAxes,
             fontsize=10,
             verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    plt.tight_layout()
    
    # Save the graph
    if output_dir is None:
        output_dir = Path(csv_path).parent
    else:
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    
    csv_name = Path(csv_path).stem
    output_path = output_dir / f"{csv_name}.png"
    
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"Graph saved: {output_path}")
    
    # Also save as PDF for high quality
    pdf_path = output_dir / f"{csv_name}.pdf"
    plt.savefig(pdf_path, format='pdf', bbox_inches='tight')
    print(f"PDF saved: {pdf_path}")
    
    plt.close()
    return True

def process_directory(directory):
    """Process all CSV files in a directory"""
    csv_files = list(Path(directory).glob("cure_log_*.csv"))
    
    if not csv_files:
        print(f"No CSV files found in {directory}")
        return
    
    print(f"Found {len(csv_files)} CSV files")
    
    for csv_file in csv_files:
        print(f"\nProcessing: {csv_file.name}")
        generate_graph(str(csv_file))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # Default to processing the cure logs directory
        log_dir = "/home/pi/cure_logs"
        print(f"Processing all logs in: {log_dir}")
        process_directory(log_dir)
    else:
        # Process specific file or directory
        path = sys.argv[1]
        if Path(path).is_file():
            generate_graph(path)
        elif Path(path).is_dir():
            process_directory(path)
        else:
            print(f"Error: {path} not found")
            sys.exit(1)