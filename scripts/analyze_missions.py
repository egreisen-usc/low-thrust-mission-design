#!/usr/bin/env python3
"""
Mission Analysis Script
Reads trajectory and comparison CSV files and generates analysis plots
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
from pathlib import Path

# Set up matplotlib style
plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['figure.figsize'] = (12, 6)
plt.rcParams['font.size'] = 10

class MissionAnalyzer:
    def __init__(self, results_dir='../results'):
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
        
        fig, ax = plt.subplots(figsize=(10, 6))
        
        # Group by thruster and get average payload fraction
        thruster_payload = self.comparison_df.groupby('Thruster')['PayloadFraction'].mean()
        
        colors = ['#2E86AB', '#A23B72', '#F18F01', '#C73E1D']
        bars = ax.bar(range(len(thruster_payload)), thruster_payload.values * 100, 
                     color=colors[:len(thruster_payload)])
        
        ax.set_xlabel('Thruster Type', fontsize=12, fontweight='bold')
        ax.set_ylabel('Average Payload Fraction (%)', fontsize=12, fontweight='bold')
        ax.set_title('Mission Efficiency: Remaining Mass After Burnout', fontsize=14, fontweight='bold')
        ax.set_xticks(range(len(thruster_payload)))
        ax.set_xticklabels(thruster_payload.index, rotation=45, ha='right')
        ax.set_ylim(0, 2)
        
        # Add value labels on bars
        for i, (bar, val) in enumerate(zip(bars, thruster_payload.values * 100)):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.05,
                   f'{val:.2f}%', ha='center', va='bottom', fontweight='bold')
        
        plt.tight_layout()
        filepath = self.results_dir / 'payload_fraction.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: payload_fraction.png")
        plt.close()
    
    def plot_trajectory_2d(self, config_name):
        """Plot 2D trajectory (XY plane)"""
        if config_name not in self.trajectories:
            print(f"Trajectory {config_name} not loaded")
            return
        
        df = self.trajectories[config_name]
        
        fig, ax = plt.subplots(figsize=(11, 11))
        
        # Plot trajectory
        x = df['r(km)'].values  # Simplified: treating r as x position
        t = df['time(s)'].values / 86400.0  # Convert to days
        
        # Color by time
        scatter = ax.scatter(x, t, c=t, cmap='viridis', s=2, alpha=0.6)
        
        ax.set_xlabel('Orbital Radius (km)', fontsize=12, fontweight='bold')
        ax.set_ylabel('Time (days)', fontsize=12, fontweight='bold')
        ax.set_title(f'Trajectory: {config_name}', fontsize=14, fontweight='bold')
        
        cbar = plt.colorbar(scatter, ax=ax)
        cbar.set_label('Time (days)', fontweight='bold')
        
        plt.tight_layout()
        base_name = config_name.replace('.yaml', '')
        filepath = self.results_dir / f'trajectory_2d_{base_name}.png'
        plt.savefig(filepath, dpi=150, bbox_inches='tight')
        print(f"✓ Saved: trajectory_2d_{base_name}.png")
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
    
    def generate_summary_report(self):
        """Generate HTML summary report with embedded plots"""
        if self.comparison_df is None:
            print("No comparison data loaded")
            return
        
        html_content = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Mission Analysis Report</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .header {
            background: linear-gradient(135deg, #2E86AB 0%, #A23B72 100%);
            color: white;
            padding: 30px;
            border-radius: 8px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        h1 {
            margin: 0;
            font-size: 2.5em;
        }
        h2 {
            color: #2E86AB;
            border-bottom: 3px solid #2E86AB;
            padding-bottom: 10px;
            margin-top: 30px;
        }
        .summary-box {
            background: white;
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .metric {
            display: inline-block;
            width: 23%;
            margin: 1%;
            padding: 15px;
            background: linear-gradient(135deg, #f5f5f5 0%, #e8e8e8 100%);
            border-radius: 6px;
            text-align: center;
            border-left: 4px solid #2E86AB;
        }
        .metric-value {
            font-size: 1.8em;
            font-weight: bold;
            color: #2E86AB;
        }
        .metric-label {
            font-size: 0.9em;
            color: #666;
            margin-top: 5px;
        }
        .plot-container {
            background: white;
            padding: 15px;
            margin: 20px 0;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        img {
            max-width: 100%;
            height: auto;
            border-radius: 6px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: white;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        th {
            background-color: #2E86AB;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: bold;
        }
        td {
            padding: 10px;
            border-bottom: 1px solid #ddd;
        }
        tr:hover {
            background-color: #f9f9f9;
        }
        .footer {
            margin-top: 40px;
            padding-top: 20px;
            border-top: 2px solid #ddd;
            text-align: center;
            color: #666;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🚀 Orbital Transfer Mission Analysis Report</h1>
        <p>Low-Thrust Electric Propulsion Mission Design Study</p>
    </div>

    <div class="summary-box">
        <h2>Executive Summary</h2>
"""
        
        if len(self.comparison_df) > 0:
            best_time = self.comparison_df.loc[self.comparison_df['FlightTime(days)'].idxmin()]
            best_dv = self.comparison_df.loc[self.comparison_df['DeltaV(km/s)'].idxmin()]
            best_efficient = self.comparison_df.loc[self.comparison_df['PayloadFraction'].idxmax()]
            
            html_content += f"""
        <p>This analysis covers <strong>{len(self.comparison_df)}</strong> mission scenarios with various electric propulsion systems.</p>
        
        <div>
            <div class="metric">
                <div class="metric-value">{best_time['FlightTime(days)']:.1f}</div>
                <div class="metric-label">Fastest Transfer (days)</div>
            </div>
            <div class="metric">
                <div class="metric-value">{best_dv['DeltaV(km/s)']:.2f}</div>
                <div class="metric-label">Minimum Delta-V (km/s)</div>
            </div>
            <div class="metric">
                <div class="metric-value">{best_efficient['PayloadFraction']*100:.2f}%</div>
                <div class="metric-label">Best Payload Fraction</div>
            </div>
            <div class="metric">
                <div class="metric-value">{self.comparison_df['FuelEfficiency(km/s/kg)'].max():.3f}</div>
                <div class="metric-label">Max Fuel Efficiency</div>
            </div>
        </div>
    </div>

    <h2>Performance Comparison Charts</h2>
    
    <div class="plot-container">
        <h3>Flight Time by Thruster Type</h3>
        <img src="flight_time_comparison.png" alt="Flight Time Comparison">
    </div>
    
    <div class="plot-container">
        <h3>Delta-V Requirements</h3>
        <img src="delta_v_comparison.png" alt="Delta-V Comparison">
    </div>
    
    <div class="plot-container">
        <h3>Fuel Efficiency Trade-off Analysis</h3>
        <img src="fuel_efficiency_tradeoff.png" alt="Fuel Efficiency">
    </div>
    
    <div class="plot-container">
        <h3>Payload Fraction by Thruster</h3>
        <img src="payload_fraction.png" alt="Payload Fraction">
    </div>

    <h2>Detailed Mission Comparison</h2>
    <div class="summary-box">
"""
        
        # Add detailed table
        html_content += """
        <table>
            <thead>
                <tr>
                    <th>Mission</th>
                    <th>Thruster</th>
                    <th>Flight Time (days)</th>
                    <th>Delta-V (km/s)</th>
                    <th>Fuel (kg)</th>
                    <th>Payload (%)</th>
                </tr>
            </thead>
            <tbody>
"""
        
        for idx, row in self.comparison_df.iterrows():
            html_content += f"""
                <tr>
                    <td>{row['Mission']}</td>
                    <td>{row['Thruster']}</td>
                    <td>{row['FlightTime(days)']:.1f}</td>
                    <td>{row['DeltaV(km/s)']:.2f}</td>
                    <td>{row['FuelConsumed(kg)']:.0f}</td>
                    <td>{row['PayloadFraction']*100:.2f}%</td>
                </tr>
"""
        
        html_content += """
            </tbody>
        </table>
    </div>

    <h2>Key Findings</h2>
    <div class="summary-box">
        <ul>
            <li>High-power thrusters achieve faster transfers but consume more fuel</li>
            <li>Low-power thrusters have better payload fractions due to lower fuel consumption</li>
            <li>Transfer efficiency approaches 100% at coast activation</li>
            <li>Fuel efficiency varies significantly with thruster type and mission profile</li>
        </ul>
    </div>

    <div class="footer">
        <p>Generated by Mission Analysis Tool | Low-Thrust Orbital Transfer Propagator v1.0</p>
    </div>
</body>
</html>
"""
        
        filepath = self.results_dir / 'mission_report.html'
        with open(filepath, 'w') as f:
            f.write(html_content)
        
        print(f"✓ Saved: mission_report.html")


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
    analyzer.generate_summary_report()
    
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
