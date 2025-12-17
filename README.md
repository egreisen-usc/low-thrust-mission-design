# Low-Thrust Mission Design: Interplanetary Trajectory Optimization
Trade tool for rapid simulation and analysis of continuous, low-thrust interplanetary transfer orbit trajectories
## Overview

This project implements a numerical trajectory propagator for low-thrust spacecraft missions. The core objective is to solve the differential equations of motion under continuous low-thrust propulsion using Runge-Kutta 4th-order (RK4) and Euler time integration methods, then compare their convergence behavior.

The application propagates spacecraft trajectories from Earth to Mars under various thruster configurations (Hall-effect and ion engines, at both high and low power settings) and performs convergence studies to validate numerical accuracy.

## Requirements

- C++17 compiler (g++ or clang)
- CMake 3.10+
- Python 3.7+ (for analysis scripts)
- Python packages: pandas, numpy, matplotlib, pyyaml

## Build Instructions

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
cd ..
```

Executables are located in `build/bin/`:
- `propagate_trajectory`: Main trajectory propagator
- `test_kepler`: Unit tests for Keplerian orbit calculations
- `test_propagation`: Unit tests for propagation algorithms

## Running Simulations

### Single Trajectory Propagation

While in root directory:
```bash
cd build
./bin/propagate_trajectory ../config/earth_mars_low_hall.yaml
```

Configuration files in `config/` define:
- Mission parameters (departure/arrival bodies, initial mass)
- Spacecraft properties (thrust, specific impulse, thruster type)
- Integration settings (method, timestep, max flight time)
- Output parameters

Results are written to `results/` as CSV files containing time-series of:
- Position and velocity (km, km/s)
- Orbital elements (a, e, rp, ra)
- Spacecraft mass and delta-V accumulated

### Batch Mission Propagation

Run multiple mission configurations sequentially and generate comparative analysis:
While in root directory:
```bash
cd build
./bin/propagate_trajectory --batch ../config/mission_batch.txt
```

The batch configuration file lists one mission config per line:

```
config/earth_mars_low_hall.yaml
config/earth_mars_low_ion.yaml
config/earth_mars_high_hall.yaml
config/earth_mars_high_ion.yaml
```

Batch mode automatically:
- Propagates all missions using the corresponding configuration files
- Extracts final orbital elements, delta-V, flight time, and fuel consumption
- Computes derived metrics (payload fraction, effective Isp, fuel efficiency)
- Generates a summary CSV (`results/mission_comparison.csv`) with all results
- Prints comparative statistics grouped by thruster type and destination

Output file contains columns:
- Mission name, Thruster type, Departure, Arrival
- Flight time (days), Total delta-V (km/s), Fuel consumed (kg), Final mass (kg)
- Apoapis, Periapsis, Eccentricity, Semi-major axis
- Payload fraction, Effective Isp, Fuel efficiency, Transfer efficiency

# Post-Processing and Visualization

After running the trajectory simulations, you can generate comparison plots and analysis visualizations using the Python analysis script.

## Quick Start

```bash
# Run post-processing analysis
cd scripts
python3 analyze_missions.py
```

All visualization plots will be saved to `results/` directory.

## Prerequisites

Install required Python packages (or set up a virtual environment):

```bash
pip install -r scripts/requirements.txt
```

Or install individually:

```bash
pip install matplotlib numpy pandas
```

## What It Generates

The script produces 8 visualization plots and an HTML summary:

| Plot | Description |
|---|---|
| `flight_time_comparison.png` | Bar chart: mission duration (days) for each thruster type |
| `delta_v_comparison.png` | Bar chart: total Δv (km/s) required for each mission |
| `fuel_efficiency_tradeoff.png` | Scatter plot: fuel consumed vs. flight time trade-off analysis |
| `payload_fraction.png` | Bar chart: final payload as % of initial mass |
| `trajectory_xy_earth_mars_*.png` (4 files) | XY position plots showing spiral trajectory for each thruster configuration |
| `orbital_elements_earth_mars_*.png` (4 files) | Orbital element evolution: apoapsis, periapsis, eccentricity over time |

## Input Data

The script reads from:
- `results/mission_comparison.csv` - Summary metrics for each mission
- `results/*_trajectory.csv` - Full trajectory data for each thruster type

## Interpreting Results

**HTML Report**
An mission_report.html is automatically generated to summarize the key findings and visualizations. You can open it in your browser or desired HTML viewer. Individual CSVs for each trajectory, as well as mission_copmarison.csv, can also be accessed from /results.

**Use the report to:**
1. Compare thruster performance across multiple metrics
2. Identify mission trade-offs (speed vs. efficiency)
3. Validate that all missions reach desired orbit successfully
4. Analyze orbital element evolution during spiral transfer

## Troubleshooting

**"No such file or directory" or missing CSV files:**
- Run the C++ simulations first (from root): `./build/bin/propagate_trajectory --batch ../config/mission_batch.txt`
- Ensure you're running the script from the `scripts/` directory

**"ModuleNotFoundError":**
- Install dependencies: `pip install -r requirements.txt`

**Plots not opening:**
- Plots are automatically saved to files, not displayed to screen
- Check `results/` directory for PNG files if there are HTML display errors

## Example Workflow

```bash
# Full workflow from build to visualization
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
cd ..

# Run all missions:
cd build
./bin/propagate_trajectory --batch ../config/mission_batch.txt

# Generate analysis plots
cd ../scripts
python3 analyze_missions.py

# View results
# Open mission_report.html in browser
```

### Convergence Study

Compares RK4 and Euler integration methods across multiple timesteps:

```bash
python3 scripts/run_convergence_comparison.py
```

Generates:
- Trajectory CSVs for each timestep in `convergence_study/results/`
- Diagnostic plots comparing convergence rates in `convergence_study/plots/`
- Console output with error analysis and convergence orders

## Project Structure

```
.
├── src/
│   ├── propagation.cpp/h       # Core ODE integration (RK4, Euler)
│   ├── kepler.cpp/h            # Orbital element calculations
│   ├── thrust_model.cpp/h      # Thruster models
│   └── main.cpp                # Entry point
├── config/
│   ├── earth_mars_low_hall.yaml
│   ├── earth_mars_low_ion.yaml
│   ├── earth_mars_high_hall.yaml
│   ├── earth_mars_high_ion.yaml
│   └── convergence_test.yaml
├── scripts/
│   └── run_convergence_comparison.py
├── results/                    # Output trajectory CSV files
└── CMakeLists.txt
```

## Configuration File Format

Example `earth_mars_low_hall.yaml`:

```yaml
mission:
  departure_body: "Earth"
  arrival_body: "Mars"
  initial_mass_kg: 10000

spacecraft:
  name: "Low-Power Hall"
  thrust_mN: 60
  isp_s: 1500

integration:
  method: "rk4"              # or "euler"
  timestep_s: 1000
  max_flight_time_s: 86400000

propagation:
  coast_threshold: 0.999

output:
  filename: my_trajectory.csv
  save_interval: 1
  print_interval: 10000
```

## Output Format

CSV files contain columns:
- `time(s)`: Mission elapsed time in seconds
- `x(km), y(km)`: Cartesian position
- `vx(km/s), vy(km/s)`: Cartesian velocity
- `r(km), v(km/s)`: Magnitude of position and velocity
- `m(kg)`: Current spacecraft mass
- `ra(km), rp(km)`: Apoapsis and periapsis
- `e`: Eccentricity
- `a(km)`: Semi-major axis

## Numerical Methods

### Runge-Kutta 4th Order (RK4)

Fourth-order explicit time integration with step size \(h\):

\[y_{n+1} = y_n + \frac{h}{6}(k_1 + 2k_2 + 2k_3 + k_4)\]

Expected local truncation error: \(\mathcal{O}(h^5)\), global error \(\mathcal{O}(h^4)\).

### Euler Method

First-order explicit method for comparison:

\[y_{n+1} = y_n + h \cdot f(t_n, y_n)\]

Expected local truncation error: \(\mathcal{O}(h^2)\), global error \(\mathcal{O}(h)\).

### Verification

Convergence is verified by:
1. Running simulations at multiple timesteps (10000, 5000, 2000, 1000 s)
2. Computing relative errors in orbital elements between solutions
3. Plotting errors on log-log scale to extract convergence order
4. Comparing observed vs. theoretical convergence rates

For low-thrust regimes where orbital changes are gradual, numerical differences may be negligible; this is documented in the convergence study results.

## Key Results

Four mission configurations have been analyzed:

| Configuration | Thruster Type | Thrust (mN) | Method | Final Delta-V (km/s) | Mission Duration |
|---|---|---|---|---|---|
| Low-Hall | Hall-effect | 60 | RK4 | ~0.24 | 1000 days |
| Low-Ion | Ion | 60 | RK4 | ~0.24 | 1000 days |
| High-Hall | Hall-effect | 450 | RK4 | ~2.97 | 75 days |
| High-Ion | Ion | 450 | RK4 | ~2.97 | 75 days |

See `results/` directory for detailed trajectory data and convergence study outputs.

## Testing

```bash
cd build
./bin/test_kepler
./bin/test_propagation
```

Tests verify:
- Kepler equation solver convergence
- Orbital element calculations
- Propagation algorithm correctness on simplified test cases

## Notes

- The coordinate system is 2D ecliptic (sun-centered inertial frame)
- Thrust direction is assumed optimal (prograde or retrograde, depending on direction of spiral)
- Coast phase is triggered when orbital energy is near target energy (99.9%)
- All values use SI-compatible units (km, s, kg, mN)
