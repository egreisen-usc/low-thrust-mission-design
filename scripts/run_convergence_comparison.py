#!/usr/bin/env python3
"""
Convergence Study: RK4 vs Euler Comparison
Demonstrates convergence behavior differences between integration methods

Usage:
    python3 scripts/run_convergence_comparison.py
    
This script:
1. Runs convergence study for BOTH RK4 and Euler methods
2. Extracts final orbital elements from each run's CSV
3. Computes relative errors vs the finest timestep for each method
4. Generates comparative diagnostic plots showing convergence order
5. Saves results to convergence_study/ directory with separate subdirs for each method
"""

import subprocess
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import yaml
import sys
import shutil
import os
import time

# Configuration
TIMESTEPS = [10000, 5000, 2000, 1000]  # seconds
EXECUTABLE = "./build/bin/propagate_trajectory"
STUDY_DIR = Path("convergence_study")
RESULTS_DIR = STUDY_DIR / "results"
PLOTS_DIR = STUDY_DIR / "plots"

# Methods to compare
METHODS = {
    'rk4': {
        'config': 'config/convergence_test.yaml',
        'name': 'RK4 (4th Order)',
        'color': '#1f77b4',
        'marker': 'o'
    },
    'euler': {
        'config': 'config/convergence_test_euler.yaml',
        'name': 'Euler (1st Order)',
        'color': '#d62728',
        'marker': 's'
    }
}

# Create directories
RESULTS_DIR.mkdir(parents=True, exist_ok=True)
PLOTS_DIR.mkdir(parents=True, exist_ok=True)

print("="*70)
print("CONVERGENCE STUDY: RK4 vs Euler Method Comparison")
print("="*70)

# Store results for both methods
convergence_data_by_method = {}

# ─────────────────────────────────────────────────────────────────────────
# PHASE 1: Run convergence study for each method
# ─────────────────────────────────────────────────────────────────────────

for method_key, method_info in METHODS.items():
    config_file = method_info['config']
    
    # Skip if config doesn't exist
    if not Path(config_file).exists():
        print(f"\n⚠ WARNING: Config file not found: {config_file}")
        print(f"   Skipping {method_key.upper()} method")
        continue
    
    print(f"\n{'='*70}")
    print(f"TESTING {method_info['name']}")
    print(f"{'='*70}")
    
    convergence_data = {}
    
    for dt in TIMESTEPS:
        print(f"\n{'─'*70}")
        print(f"Running {method_key.upper()} with Δt = {dt:,} s")
        print(f"{'─'*70}")
        
        # Create subdirectory for this timestep and method
        dt_dir = RESULTS_DIR / f"{method_key}_timestep_{dt}"
        dt_dir.mkdir(exist_ok=True)
        
        with open(config_file, 'r') as f:
            config = yaml.safe_load(f)
        
        # Override timestep using correct YAML key: timestep_s
        config['integration']['timestep_s'] = dt
        
        # Write modified config to temp file
        temp_config = dt_dir / "config_modified.yaml"
        with open(temp_config, 'w') as f:
            yaml.dump(config, f)
        
        # Run propagator with timestep override
        cmd = [EXECUTABLE, str(temp_config), "--timestep", str(dt)]
        
        print(f"Command: {' '.join(cmd)}")
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=300
            )
            
            if result.returncode != 0:
                print(f"❌ ERROR: Propagator failed (exit code {result.returncode})")
                if result.stderr:
                    print(f"   stderr: {result.stderr[:500]}")
                continue
            
            print(f"✓ Propagator completed successfully")
            
            # Find trajectory CSV files
            csv_files = list(Path("results").glob("*trajectory.csv"))
            
            if not csv_files:
                print(f"❌ ERROR: No CSV output found in results/")
                continue
            
            # Copy all CSVs to timestep directory
            for csv_file in csv_files:
                output_csv = dt_dir / csv_file.name
                shutil.copy(csv_file, output_csv)
                print(f"✓ Copied: {csv_file.name}")
                
                # Parse first trajectory file
                if csv_file == csv_files[0]:
                    try:
                        df = pd.read_csv(output_csv)
                    except Exception as e:
                        print(f"❌ ERROR: Could not parse CSV: {e}")
                        continue
                    
                    final_row = df.iloc[-1]
                    
                    # Extract values using actual column names
                    convergence_data[dt] = {
                        'steps': len(df),
                        'time': final_row.get('time(s)', np.nan),
                        'a': final_row.get('a(km)', np.nan),
                        'e': final_row.get('e', np.nan),
                        'r_p': final_row.get('rp(km)', np.nan),
                        'r_a': final_row.get('ra(km)', np.nan),
                        'fuel': final_row.get('m(kg)', np.nan),
                        'csv_file': str(output_csv)
                    }
                    
                    print(f"\n   Final orbital elements:")
                    print(f"   a  = {convergence_data[dt]['a']:>15.2f} km")
                    print(f"   e  = {convergence_data[dt]['e']:>15.8f}")
                    print(f"   r_p = {convergence_data[dt]['r_p']:>14.2f} km")
                    print(f"   r_a = {convergence_data[dt]['r_a']:>14.2f} km")
                    print(f"   Steps: {convergence_data[dt]['steps']:>11}")
                
        except subprocess.TimeoutExpired:
            print(f"❌ ERROR: Propagator timeout (>300s)")
        except Exception as e:
            print(f"❌ ERROR: {e}")
    
    if not convergence_data:
        print(f"\n⚠ WARNING: No convergence data collected for {method_key}")
        continue
    
    convergence_data_by_method[method_key] = convergence_data

if not convergence_data_by_method:
    print("\n" + "="*70)
    print("❌ No convergence data collected for any method. Exiting.")
    print("="*70)
    sys.exit(1)

# ─────────────────────────────────────────────────────────────────────────
# PHASE 2: Compute errors and convergence metrics for each method
# ─────────────────────────────────────────────────────────────────────────

print(f"\n{'='*70}")
print("CONVERGENCE ANALYSIS")
print(f"{'='*70}")

error_data_by_method = {}

for method_key in convergence_data_by_method:
    convergence_data = convergence_data_by_method[method_key]
    method_info = METHODS[method_key]
    
    print(f"\n{'─'*70}")
    print(f"Analysis for {method_info['name']}")
    print(f"{'─'*70}")
    
    # Check for valid data
    valid_timesteps = [dt for dt in convergence_data if not np.isnan(convergence_data[dt]['a'])]
    if not valid_timesteps:
        print("❌ No valid data for analysis")
        continue
    
    # Use finest valid timestep as reference
    dt_ref = max(valid_timesteps)
    a_ref = convergence_data[dt_ref]['a']
    e_ref = convergence_data[dt_ref]['e']
    rp_ref = convergence_data[dt_ref]['r_p']
    ra_ref = convergence_data[dt_ref]['r_a']
    
    print(f"\nReference solution (Δt = {dt_ref:,} s):")
    print(f"  a   = {a_ref:>15.2f} km")
    print(f"  e   = {e_ref:>15.8f}")
    print(f"  r_p = {rp_ref:>15.2f} km")
    print(f"  r_a = {ra_ref:>15.2f} km")
    
    # Compute relative errors
    error_data = {}
    print(f"\nRelative errors (vs Δt = {dt_ref:,} s):")
    print(f"{'Δt (s)':>10} | {'Δa (%)':>12} | {'Δe (%)':>12} | {'Δrp (%)':>12}")
    print("─" * 62)
    
    for dt in sorted(valid_timesteps):
        error_a = abs(convergence_data[dt]['a'] - a_ref) / a_ref * 100
        error_e = abs(convergence_data[dt]['e'] - e_ref) / abs(e_ref) * 100 if e_ref != 0 else 0
        error_rp = abs(convergence_data[dt]['r_p'] - rp_ref) / rp_ref * 100 if rp_ref != 0 else 0
        
        error_data[dt] = {'a': error_a, 'e': error_e, 'rp': error_rp}
        
        print(f"{dt:>10} | {error_a:>12.6f} | {error_e:>12.6f} | {error_rp:>12.6f}")
    
    # Compute convergence order
    print(f"\nConvergence order (log-log slope):")
    print(f"{'Δt ratio':>15} | {'a order':>12} | {'e order':>12} | {'rp order':>12}")
    print("─" * 62)
    
    sorted_dts = sorted(valid_timesteps)
    for i in range(1, len(sorted_dts)):
        dt_prev = sorted_dts[i-1]
        dt_curr = sorted_dts[i]
        
        err_a_prev = error_data[dt_prev]['a']
        err_a_curr = error_data[dt_curr]['a']
        err_e_prev = error_data[dt_prev]['e']
        err_e_curr = error_data[dt_curr]['e']
        err_rp_prev = error_data[dt_prev]['rp']
        err_rp_curr = error_data[dt_curr]['rp']
        
        if err_a_prev > 1e-6 and err_a_curr > 1e-6:
            order_a = np.log(err_a_prev / err_a_curr) / np.log(dt_prev / dt_curr)
        else:
            order_a = np.nan
        
        if err_e_prev > 1e-6 and err_e_curr > 1e-6:
            order_e = np.log(err_e_prev / err_e_curr) / np.log(dt_prev / dt_curr)
        else:
            order_e = np.nan
        
        if err_rp_prev > 1e-6 and err_rp_curr > 1e-6:
            order_rp = np.log(err_rp_prev / err_rp_curr) / np.log(dt_prev / dt_curr)
        else:
            order_rp = np.nan
        
        ratio = dt_prev / dt_curr
        print(f"{ratio:>15.1f} | {order_a:>12.2f} | {order_e:>12.2f} | {order_rp:>12.2f}")
    
    error_data_by_method[method_key] = {
        'valid_timesteps': valid_timesteps,
        'error_data': error_data,
        'dt_ref': dt_ref,
        'a_ref': a_ref,
        'e_ref': e_ref,
        'rp_ref': rp_ref
    }

# ─────────────────────────────────────────────────────────────────────────
# PHASE 3: Generate comparative diagnostic plots
# ─────────────────────────────────────────────────────────────────────────

print(f"\nGenerating comparative plots...")

# Figure 1: Semi-major Axis Convergence Comparison
fig, ax = plt.subplots(figsize=(12, 7))

for method_key in convergence_data_by_method:
    method_info = METHODS[method_key]
    convergence_data = convergence_data_by_method[method_key]
    valid_timesteps = error_data_by_method[method_key]['valid_timesteps']
    
    dt_array = np.array(sorted(valid_timesteps))
    a_array = np.array([convergence_data[dt]['a'] for dt in dt_array])
    
    ax.semilogx(dt_array, a_array, marker=method_info['marker'], linewidth=2.5, 
                markersize=10, color=method_info['color'], label=method_info['name'])

ax.set_xlabel('Timestep Δt (s)', fontsize=12, fontweight='bold')
ax.set_ylabel('Semi-major Axis a (km)', fontsize=12, fontweight='bold')
ax.set_title('Semi-major Axis: RK4 vs Euler Convergence', fontsize=13, fontweight='bold')
ax.legend(fontsize=11, loc='best')
ax.grid(True, alpha=0.3, linestyle='--')
ax.set_axisbelow(True)

plt.tight_layout()
plt.savefig(PLOTS_DIR / 'convergence_comparison_orbital_elements.png', dpi=300, bbox_inches='tight')
print(f"✓ Saved: convergence_comparison_orbital_elements.png")
plt.close()

# Figure 2: Relative Error vs Timestep (Log-Log Comparison)
fig, ax = plt.subplots(figsize=(12, 8))

for method_key in convergence_data_by_method:
    method_info = METHODS[method_key]
    convergence_data = convergence_data_by_method[method_key]
    error_data = error_data_by_method[method_key]['error_data']
    valid_timesteps = error_data_by_method[method_key]['valid_timesteps']
    
    dt_array = np.array(sorted(valid_timesteps))
    error_a_array = np.array([error_data[dt]['a'] for dt in dt_array])
    
    # Plot errors (exclude finest, which has zero error)
    dt_plot = dt_array[:-1]
    error_a_plot = error_a_array[:-1]
    
    if len(error_a_plot) > 0 and max(error_a_plot) > 0:
        ax.loglog(dt_plot, error_a_plot, marker=method_info['marker'], linewidth=2.5, 
                  markersize=10, color=method_info['color'], label=method_info['name'])

# Add reference convergence rate lines
if len(convergence_data_by_method) > 0:
    # Get timestep range from first method
    first_method = list(convergence_data_by_method.keys())[0]
    valid_ts = error_data_by_method[first_method]['valid_timesteps']
    sorted_dts = sorted(valid_ts)
    
    if len(sorted_dts) >= 2:
        dt_min, dt_max = sorted_dts[0], sorted_dts[-1]
        
        x_ref = np.logspace(np.log10(dt_min), np.log10(dt_max), 100)
        
        # Find a reasonable error value for scaling reference lines
        err_vals = []
        for method_key in convergence_data_by_method:
            error_data = error_data_by_method[method_key]['error_data']
            for dt in error_data:
                err_vals.append(error_data[dt]['a'])
        
        if err_vals and max(err_vals) > 1e-6:
            err_max = max(err_vals)
            nonzero_errs = [e for e in err_vals if e > 1e-6]
            if nonzero_errs:
                err_min = min(nonzero_errs)
                
                # O(Δt) line (first-order, Euler expected)
                scale_1 = err_max
                y_linear = scale_1 * (x_ref / dt_max)**1
                ax.loglog(x_ref, y_linear, 'k--', alpha=0.4, linewidth=1.5, label='O(Δt) reference')
                
                # O(Δt⁴) line (fourth-order, RK4 expected)
                scale_4 = err_max * (dt_max / dt_min)**3  # Scale to be visible
                y_quart = scale_4 * (x_ref / dt_max)**4
                ax.loglog(x_ref, y_quart, 'k-.', alpha=0.4, linewidth=1.5, label='O(Δt⁴) reference')

ax.set_xlabel('Timestep Δt (s)', fontsize=12, fontweight='bold')
ax.set_ylabel('Relative Error in Semi-major Axis (%)', fontsize=12, fontweight='bold')
ax.set_title('Convergence Rate Comparison: RK4 vs Euler', fontsize=13, fontweight='bold')
ax.legend(fontsize=11, loc='upper left', framealpha=0.95)
ax.grid(True, which='both', alpha=0.3, linestyle='--')
ax.set_axisbelow(True)

plt.tight_layout()
plt.savefig(PLOTS_DIR / 'convergence_comparison_error_rate.png', dpi=300, bbox_inches='tight')
print(f"✓ Saved: convergence_comparison_error_rate.png")
plt.close()

# ─────────────────────────────────────────────────────────────────────────
# PHASE 4: Summary and Interpretation
# ─────────────────────────────────────────────────────────────────────────

print(f"\n{'='*70}")
print("CONVERGENCE STUDY COMPLETE")
print(f"{'='*70}")
print(f"\nResults saved to: {STUDY_DIR}/")
print(f"  • Trajectory CSVs: results/rk4_timestep_*/ and results/euler_timestep_*/")
print(f"  • Comparative plots: plots/")

print(f"\n{'='*70}")
print("INTERPRETATION")
print(f"{'='*70}\n")

for method_key in sorted(convergence_data_by_method.keys()):
    method_info = METHODS[method_key]
    error_data = error_data_by_method[method_key]['error_data']
    valid_timesteps = error_data_by_method[method_key]['valid_timesteps']
    
    dt_coarse = min(valid_timesteps)
    error_coarse_a = error_data[dt_coarse]['a']
    
    print(f"{method_info['name']}:")
    
    if error_coarse_a < 0.1:
        status = "✓ EXCELLENT"
        msg = f"Error at Δt = {dt_coarse:,}s is < 0.1%"
    elif error_coarse_a < 1.0:
        status = "✓ GOOD"
        msg = f"Error at Δt = {dt_coarse:,}s is < 1.0%"
    elif error_coarse_a < 5.0:
        status = "⚠ ACCEPTABLE"
        msg = f"Error at Δt = {dt_coarse:,}s is < 5.0%"
    else:
        status = "⚠ WARNING"
        msg = f"Error at Δt = {dt_coarse:,}s exceeds 5.0%"
    
    print(f"  {status}: {msg}")
    
    # Show convergence order
    sorted_dts = sorted(valid_timesteps)
    if len(sorted_dts) >= 2:
        dt_prev = sorted_dts[0]
        dt_curr = sorted_dts[1]
        err_prev = error_data[dt_prev]['a']
        err_curr = error_data[dt_curr]['a']
        if err_prev > 1e-6 and err_curr > 1e-6:
            order = np.log(err_prev / err_curr) / np.log(dt_prev / dt_curr)
            print(f"  Observed convergence order (first step): {order:.2f}")
    
    print()

print(f"{'='*70}\n")
