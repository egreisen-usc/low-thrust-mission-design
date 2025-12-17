#!/usr/bin/env python3
"""
Mission Analysis Script
Reads trajectory and comparison CSV files and generates analysis plots
"""

import pandas as pd
import matplotlib
matplotlib.use("Agg")  # Use non-GUI backend
import matplotlib.pyplot as plt
import numpy as np
import os
from pathlib import Path

# Set up matplotlib style
plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['figure.figsize'] = (12, 6)
plt.rcParams['font.size'] = 10

# Approximate circular orbit radii around the Sun (km)
PLANET_RADII_KM = {
    "Mercury": 5.791e7,
    "Venus":   1.082e8,
    "Earth":   1.496e8,
    "Mars":    2.279e8,
    "Jupiter": 7.785e8,
    "Saturn":  1.433e9,
    "Uranus":  2.877e9,
    "Neptune": 4.503e9
}

class MissionAnalyzer:
    def __init__(self, results_dir='./results'):
        """Initialize the analyzer with results directory"""
        self.results_dir = Path(results_dir)
        self.results_dir.mkdir(exist_ok=True)
        self.comparison_df = None
        self.trajectories = {}
        
    def load_comparison(self, filename='mission_comparison.csv'):
        """Load mission comparison CSV"""
        filepath = self.results_dir / filename
        if filepath.exists():
            self.comparison_df = pd.read_csv(filepath)
            print(f"✓ Loaded comparison data: {filename}")
            print(f"  Missions: {len(self.comparison_df)}")
            return True
        else:
            print(f"✗ Comparison file not found: {filepath}")
            return False
    
    def load_trajectory(self, config_name):
        """Load a single trajectory CSV"""
        # Extract base name without extension
        base_name = config_name.replace('.yaml', '')
        filename = f"{base_name}_trajectory.csv"
        filepath = self.results_dir / filename
        
        if filepath.exists():
            df = pd.read_csv(filepath)
            self.trajectories[config_name] = df
            print(f"✓ Loaded trajectory: {filename}")
            return True
        else:
            print(f"✗ Trajectory file not found: {filepath}")
            return False
    
    def plot_flight_time_comparison(self):
        """Create bar chart of flight times by thruster"""
        if self.comparison_df is None:
            print("No comparison data loaded")
            return
        
        fig, ax = plt.subplots(figsize=(10, 6))
        
        # Group by thruster and get average flight time
        thruster_times = self.comparison_df.groupby('Thruster')['FlightTime(days)'].mean()
        
        colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D']
        bars = ax.bar(range(len(thruster_times)), thruster_times.values, color=colors[:len(thruster_times)])
        
        ax.set_xlabel('Thruster Type', fontsize=12, fontweight='bold')
        ax.set_ylabel('Average Flight Time (days)', fontsize=12, fontweight='bold')
        ax.set_title('Mission Duration by Thruster Type', fontsize=14, fontweight='bold')
        ax.set_xticks(range(len(thruster_times)))
        ax.set_xticklabels(thruster_times.index, rotation=45, ha='right')
        
        # Add value labels on bars
        for i, (bar, val) in enumerate(zip(bars, thruster_times.values)):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 10,
                   f'{val:.1f}', ha='center', va='bottom', fontweight='bold')
        
        plt.tight_layout()
        filepath = self.results_dir / 'flight_time_comparison.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: flight_time_comparison.png")
        plt.close()
    
    def plot_delta_v_comparison(self):
        """Create bar chart of delta-V by thruster"""
        if self.comparison_df is None:
            print("No comparison data loaded")
            return
        
        fig, ax = plt.subplots(figsize=(10, 6))
        
        # Group by thruster and get average delta-V
        thruster_dv = self.comparison_df.groupby('Thruster')['DeltaV(km/s)'].mean()
        
        colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D']
        bars = ax.bar(range(len(thruster_dv)), thruster_dv.values, color=colors[:len(thruster_dv)])
        
        ax.set_xlabel('Thruster Type', fontsize=12, fontweight='bold')
        ax.set_ylabel('Average Delta-V (km/s)', fontsize=12, fontweight='bold')
        ax.set_title('Velocity Change Required by Thruster Type', fontsize=14, fontweight='bold')
        ax.set_xticks(range(len(thruster_dv)))
        ax.set_xticklabels(thruster_dv.index, rotation=45, ha='right')
        
        # Add value labels on bars
        for i, (bar, val) in enumerate(zip(bars, thruster_dv.values)):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5,
                   f'{val:.2f}', ha='center', va='bottom', fontweight='bold')
        
        plt.tight_layout()
        filepath = self.results_dir / 'delta_v_comparison.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: delta_v_comparison.png")
        plt.close()
    
    def plot_fuel_efficiency(self):
        """Create scatter plot of fuel efficiency vs flight time"""
        if self.comparison_df is None:
            print("No comparison data loaded")
            return
        
        fig, ax = plt.subplots(figsize=(11, 7))
        
        # Create scatter plot colored by thruster
        thrusters = self.comparison_df['Thruster'].unique()
        colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D']
        
        for thruster, color in zip(thrusters, colors):
            mask = self.comparison_df['Thruster'] == thruster
            data = self.comparison_df[mask]
            ax.scatter(data['FlightTime(days)'], data['FuelEfficiency(km/s/kg)'],
                      label=thruster, s=150, alpha=0.7, color=color, edgecolors='black', linewidth=1.5)
        
        ax.set_xlabel('Flight Time (days)', fontsize=12, fontweight='bold')
        ax.set_ylabel('Fuel Efficiency (km/s/kg)', fontsize=12, fontweight='bold')
        ax.set_title('Fuel Efficiency vs Flight Time Trade-off', fontsize=14, fontweight='bold')
        ax.legend(title='Thruster Type', fontsize=10, title_fontsize=11)
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        filepath = self.results_dir / 'fuel_efficiency_tradeoff.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: fuel_efficiency_tradeoff.png")
        plt.close()
    
    def plot_payload_fraction(self):
        """Create bar chart of payload fraction by thruster"""
        if self.comparison_df is None:
            print("No comparison data loaded")
            return
        
        # Wider, shorter figure
        fig, ax = plt.subplots(figsize=(8, 4))
        
        thruster_payload = self.comparison_df.groupby('Thruster')['PayloadFraction'].mean()
        
        colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D']
        bars = ax.bar(range(len(thruster_payload)),
                    thruster_payload.values * 100,
                    color=colors[:len(thruster_payload)])
        
        ax.set_xlabel('Thruster Type', fontsize=11, fontweight='bold')
        ax.set_ylabel('Average Payload Fraction (%)', fontsize=11, fontweight='bold')
        ax.set_title('Remaining Mass After Burnout', fontsize=13, fontweight='bold')
        ax.set_xticks(range(len(thruster_payload)))
        ax.set_xticklabels(thruster_payload.index, rotation=30, ha='right')
        
        # Auto y-limits with a bit of headroom
        ymax = (thruster_payload.values * 100).max()
        ax.set_ylim(0, ymax * 1.2)
        
        for bar, val in zip(bars, thruster_payload.values * 100):
            ax.text(bar.get_x() + bar.get_width()/2,
                    val + ymax * 0.03,
                    f'{val:.2f}%',
                    ha='center', va='bottom', fontweight='bold', fontsize=9)
        
        plt.tight_layout()
        filepath = self.results_dir / 'payload_fraction.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: payload_fraction.png")
        plt.close()
    
    def plot_trajectory_2d(self, config_name):
        """Plot true 2D trajectory in XY plane using propagated state, for any planet pair."""
        if config_name not in self.trajectories:
            print(f"Trajectory {config_name} not loaded")
            return
        
        if self.comparison_df is None:
            print("No comparison data loaded")
            return
        
        df = self.trajectories[config_name]
        
        # Find the row in comparison_df corresponding to this mission
        row = self.comparison_df[self.comparison_df['Mission'] == config_name]
        if row.empty:
            print(f"No comparison entry found for mission {config_name}")
            return
        
        dep = row['From'].iloc[0]
        arr = row['To'].iloc[0]
        
        # Radii in km (fallback: None if not in dict)
        r_dep = PLANET_RADII_KM.get(dep, None)
        r_arr = PLANET_RADII_KM.get(arr, None)
        
        # Trajectory state
        x = df['x(km)'].values / 1e6
        y = df['y(km)'].values / 1e6
        t_days = df['time(s)'].values / 86400.0
        
        fig, ax = plt.subplots(figsize=(10, 10))
        
        sc = ax.scatter(x, y, c=t_days, cmap='viridis', s=3, alpha=0.8)
        
        # Start and end markers
        ax.plot(x[0],  y[0],  'go', markersize=8, label='Start', zorder=5)
        ax.plot(x[-1], y[-1], 'rs', markersize=8, label='End (Coast)', zorder=5)
        
        # Sun
        ax.plot(0, 0, 'y*', markersize=14, label='Sun', zorder=6)
        
        # Reference orbits if known
        theta = np.linspace(0, 2*np.pi, 400)
        if r_dep is not None:
            r_dep_m = r_dep / 1e6
            ax.plot(r_dep_m*np.cos(theta), r_dep_m*np.sin(theta),
                    'b--', linewidth=1.3, label=f'{dep} Orbit')
        if r_arr is not None:
            r_arr_m = r_arr / 1e6
            ax.plot(r_arr_m*np.cos(theta), r_arr_m*np.sin(theta),
                    'r--', linewidth=1.3, label=f'{arr} Orbit')
        
        ax.set_xlabel('X Position (million km)', fontsize=12, fontweight='bold')
        ax.set_ylabel('Y Position (million km)', fontsize=12, fontweight='bold')
        ax.set_title(f'Spiral Transfer Trajectory: {dep} → {arr}\nMission: {config_name}',
                    fontsize=14, fontweight='bold')
        ax.set_aspect('equal', 'box')
        ax.grid(True, alpha=0.3)
        ax.legend(loc='upper right', fontsize=9)
        
        cbar = plt.colorbar(sc, ax=ax)
        cbar.set_label('Time (days)', fontweight='bold')
        
        plt.tight_layout()
        base_name = config_name.replace('.yaml', '')
        filepath = self.results_dir / f'trajectory_xy_{base_name}.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: trajectory_xy_{base_name}.png")
        plt.close()

    def plot_orbital_elements_evolution(self, config_name):
        """Plot how orbital elements change over time"""
        if config_name not in self.trajectories:
            print(f"Trajectory {config_name} not loaded")
            return
        
        df = self.trajectories[config_name]
        t = df['time(s)'].values / 86400.0  # Convert to days
        
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(13, 10))
        
        # Apoapsis evolution
        ax1.plot(t, df['ra(km)'].values / 1e6, linewidth=2, color='#2E86AB')
        ax1.set_ylabel('Apoapsis (million km)', fontweight='bold')
        ax1.set_title('Apoapsis Evolution', fontweight='bold')
        ax1.grid(True, alpha=0.3)
        
        # Periapsis evolution
        ax2.plot(t, df['rp(km)'].values / 1e6, linewidth=2, color='#A23B72')
        ax2.set_ylabel('Periapsis (million km)', fontweight='bold')
        ax2.set_title('Periapsis Evolution', fontweight='bold')
        ax2.grid(True, alpha=0.3)
        
        # Eccentricity evolution
        ax3.plot(t, df['e'].values, linewidth=2, color='#F18F01')
        ax3.set_xlabel('Time (days)', fontweight='bold')
        ax3.set_ylabel('Eccentricity', fontweight='bold')
        ax3.set_title('Eccentricity Evolution', fontweight='bold')
        ax3.grid(True, alpha=0.3)
        
        # Semi-major axis evolution
        ax4.plot(t, df['a(km)'].values / 1e6, linewidth=2, color='#C73E1D')
        ax4.set_xlabel('Time (days)', fontweight='bold')
        ax4.set_ylabel('Semi-major axis (million km)', fontweight='bold')
        ax4.set_title('Semi-major Axis Evolution', fontweight='bold')
        ax4.grid(True, alpha=0.3)
        
        plt.tight_layout()
        base_name = config_name.replace('.yaml', '')
        filepath = self.results_dir / f'orbital_elements_{base_name}.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: orbital_elements_{base_name}.png")
        plt.close()
    
    def generate_html_report(self):
        """Generate comprehensive HTML report with all plots"""
        if self.comparison_df is None:
            print("No comparison data to generate report")
            return

        # Get mission names for organized sections
        missions = self.comparison_df['Mission'].unique()
        
        html_content = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Low-Thrust Orbital Transfer Analysis Report</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f5f5f5;
            color: #333;
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        header {
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            color: white;
            padding: 40px 20px;
            text-align: center;
            margin-bottom: 40px;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        
        header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        header p {
            font-size: 1.1em;
            opacity: 0.9;
        }
        
        section {
            background: white;
            margin-bottom: 30px;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        section h2 {
            color: #1e3c72;
            border-bottom: 3px solid #2a5298;
            padding-bottom: 15px;
            margin-bottom: 25px;
            font-size: 1.8em;
        }
        
        section h3 {
            color: #2a5298;
            margin-top: 20px;
            margin-bottom: 15px;
            font-size: 1.3em;
        }
        
        .image-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(500px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .image-grid-full {
            display: grid;
            grid-template-columns: 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .image-container {
            background: #f9f9f9;
            padding: 15px;
            border-radius: 6px;
            border: 1px solid #e0e0e0;
            transition: all 0.3s ease;
        }
        
        .image-container:hover {
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
            transform: translateY(-2px);
        }
        
        .image-container img {
            width: 100%;
            height: auto;
            border-radius: 4px;
            display: block;
        }
        
        .image-title {
            text-align: center;
            margin-top: 10px;
            font-weight: 600;
            color: #1e3c72;
            font-size: 0.95em;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
            font-size: 0.95em;
        }
        
        table thead {
            background-color: #2a5298;
            color: white;
        }
        
        table th {
            padding: 12px;
            text-align: left;
            font-weight: 600;
        }
        
        table td {
            padding: 10px 12px;
            border-bottom: 1px solid #e0e0e0;
        }
        
        table tbody tr:nth-child(even) {
            background-color: #f5f5f5;
        }
        
        table tbody tr:hover {
            background-color: #efefef;
        }
        
        .toc {
            background: #f0f4f8;
            padding: 20px;
            border-radius: 6px;
            margin-bottom: 20px;
        }
        
        .toc h3 {
            margin-top: 0;
        }
        
        .toc ul {
            list-style: none;
            padding-left: 20px;
        }
        
        .toc li {
            margin-bottom: 8px;
        }
        
        .toc a {
            color: #2a5298;
            text-decoration: none;
            transition: color 0.2s;
        }
        
        .toc a:hover {
            color: #1e3c72;
            text-decoration: underline;
        }
        
        footer {
            text-align: center;
            padding: 20px;
            color: #999;
            font-size: 0.9em;
            border-top: 1px solid #e0e0e0;
            margin-top: 40px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>Low-Thrust Orbital Transfer</h1>
            <p>Mission Analysis and Comparative Study Report</p>
            <p>Trajectory Optimization and Performance Analysis</p>
        </header>
        
        <!-- Table of Contents -->
        <section>
            <div class="toc">
                <h3>Contents</h3>
                <ul>
                    <li><a href="#overview">Executive Summary</a></li>
                    <li><a href="#comparison">Mission Comparison</a></li>
                    <li><a href="#trajectories">Trajectory Analysis</a></li>
                    <li><a href="#orbital-elements">Orbital Element Analysis</a></li>
                </ul>
            </div>
        </section>
        
        <!-- Executive Summary -->
        <section id="overview">
            <h2>Executive Summary</h2>
            <p>This report presents a comprehensive analysis of low-thrust orbital transfer missions using multiple thruster configurations.</p>
            
            <h3>Mission Data</h3>
            <table>
                <thead>
                    <tr>
                        <th>Mission</th>
                        <th>Thruster Type</th>
                        <th>Flight Time (days)</th>
                        <th>Delta-V (km/s)</th>
                        <th>Fuel Consumed (kg)</th>
                        <th>Payload Fraction</th>
                    </tr>
                </thead>
                <tbody>
"""
        
        # Add mission data rows
        for idx, row in self.comparison_df.iterrows():
            html_content += f"""
                    <tr>
                        <td>{row['Mission']}</td>
                        <td>{row['Thruster']}</td>
                        <td>{row['FlightTime(days)']:.1f}</td>
                        <td>{row['DeltaV(km/s)']:.2f}</td>
                        <td>{row['FuelConsumed(kg)']:.2f}</td>
                        <td>{row['PayloadFraction']*100:.2f}%</td>
                    </tr>
"""
        
        html_content += """
                </tbody>
            </table>
        </section>
        
        <!-- Comparison Plots Section -->
        <section id="comparison">
            <h2>Mission Comparison</h2>
            <p>The following plots compare performance metrics across all analyzed missions:</p>
            
            <div class="image-grid">
                <div class="image-container">
                    <img src="flight_time_comparison.png" alt="Flight Time Comparison">
                    <div class="image-title">Mission Duration by Configuration</div>
                </div>
                <div class="image-container">
                    <img src="delta_v_comparison.png" alt="Delta-V Comparison">
                    <div class="image-title">Velocity Change Required</div>
                </div>
                <div class="image-container">
                    <img src="fuel_efficiency_tradeoff.png" alt="Fuel Efficiency Trade-off">
                    <div class="image-title">Efficiency vs Duration</div>
                </div>
                <div class="image-container">
                    <img src="payload_fraction.png" alt="Payload Fraction">
                    <div class="image-title">Payload Mass Fraction</div>
                </div>
            </div>
        </section>
        
        <!-- Individual Mission Trajectories -->
        <section id="trajectories">
            <h2>Trajectory Analysis</h2>
            <p>The following plots show the computed transfer trajectories for each mission configuration:</p>
"""
        
        # Add trajectory plots
        for mission in missions:
            base_name = mission.replace('.yaml', '')
            html_content += f"""
            <h3>{mission}</h3>
            <div class="image-grid-full">
                <div class="image-container">
                    <img src="trajectory_xy_{base_name}.png" alt="Trajectory: {mission}">
                    <div class="image-title">Spiral Transfer Trajectory: {mission}</div>
                </div>
            </div>
"""
        
        html_content += """
        </section>
        
        <!-- Orbital Elements Evolution -->
        <section id="orbital-elements">
            <h2>Orbital Element Analysis</h2>
            <p>The following plots show the evolution of orbital elements during each transfer:</p>
"""
        
        # Add orbital elements plots
        for mission in missions:
            base_name = mission.replace('.yaml', '')
            html_content += f"""
            <h3>{mission}</h3>
            <div class="image-grid-full">
                <div class="image-container">
                    <img src="orbital_elements_{base_name}.png" alt="Orbital Elements: {mission}">
                    <div class="image-title">Orbital Element Evolution: {mission}</div>
                </div>
            </div>
"""
        
        html_content += """
        </section>
        
        <!-- Footer -->
        <footer>
            <p>Low-Thrust Orbital Transfer Analysis Report</p>
            <p>Generated by Mission Analysis Script</p>
        </footer>
    </div>
</body>
</html>
"""
        
        # Write HTML file
        filepath = self.results_dir / 'mission_report.html'
        with open(filepath, 'w') as f:
            f.write(html_content)
        
        print(f"✓ Generated HTML report: {filepath}")
        return str(filepath)

def main():
    """Main analysis script"""
    print("\n" + "="*60)
    print("ORBITAL TRANSFER MISSION ANALYSIS")
    print("="*60 + "\n")
    
    analyzer = MissionAnalyzer()
    
    # Load comparison data
    print("Loading data...")
    if not analyzer.load_comparison():
        print("Cannot proceed without comparison data")
        return
    
    # Load individual trajectories
    for mission in analyzer.comparison_df['Mission'].unique():
        analyzer.load_trajectory(mission)
    
    print("\nGenerating analysis plots...")
    
    # Generate comparison plots
    analyzer.plot_flight_time_comparison()
    analyzer.plot_delta_v_comparison()
    analyzer.plot_fuel_efficiency()
    analyzer.plot_payload_fraction()
    
        # Generate trajectory-specific plots
    print("\nGenerating trajectory-specific plots...")
    for mission in analyzer.comparison_df['Mission'].unique():
        if mission in analyzer.trajectories:
            analyzer.plot_trajectory_2d(mission)
            analyzer.plot_orbital_elements_evolution(mission)
    
    # Generate summary report
    print("\nGenerating HTML report...")
    analyzer.generate_html_report()
    
    print("\n" + "="*60)
    print("Analysis Complete!")
    print("="*60)
    print("\nGenerated files:")
    print("  - flight_time_comparison.png")
    print("  - delta_v_comparison.png")
    print("  - fuel_efficiency_tradeoff.png")
    print("  - payload_fraction.png")
    print("  - trajectory_2d_*.png")
    print("  - orbital_elements_*.png")
    print("  - mission_report.html")
    print("\nOpen 'mission_report.html' in your browser to view the report\n")


if __name__ == '__main__':
    main()
