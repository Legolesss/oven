#!/usr/bin/env bash
# Setup script for oven data logging system

set -euo pipefail

echo "=== Oven Data Logging Setup ==="
echo

# Resolve paths based on the current user
HOME_DIR="${HOME}"
LOG_DIR="${HOME_DIR}/cure_logs"
SCRIPT_DIR="${HOME_DIR}/scripts"
BASHRC="${HOME_DIR}/.bashrc"
DESKTOP_DIR="${HOME_DIR}/Desktop"

echo "Using HOME: ${HOME_DIR}"
echo "Log directory: ${LOG_DIR}"
echo "Script directory: ${SCRIPT_DIR}"
echo

# Create directories
echo "Creating log directory: ${LOG_DIR}"
mkdir -p "${LOG_DIR}"
chmod 755 "${LOG_DIR}"

echo "Creating scripts directory: ${SCRIPT_DIR}"
mkdir -p "${SCRIPT_DIR}"

# Figure out where we're running from (assume repo root)
REPO_SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Install the graph generator script (from the repo, not /home/claude)
if [ -f "${REPO_SCRIPTS_DIR}/generate_graphs.py" ]; then
  echo "Installing graph generator script..."
  cp "${REPO_SCRIPTS_DIR}/generate_graphs.py" "${SCRIPT_DIR}/"
  chmod +x "${SCRIPT_DIR}/generate_graphs.py"
else
  echo "WARNING: ${REPO_SCRIPTS_DIR}/generate_graphs.py not found"
fi

# Install Python dependencies
echo
echo "Installing Python dependencies..."
if command -v pip3 >/dev/null 2>&1; then
  if pip3 install --break-system-packages pandas matplotlib; then
    true
  else
    echo "WARNING: Could not install Python packages. Try:"
    echo "  pip3 install --break-system-packages pandas matplotlib"
  fi
else
  echo "WARNING: pip3 not found; install python3-pip first."
fi

# Create convenience aliases
if ! grep -q "alias plot_cure=" "${BASHRC}" 2>/dev/null; then
  echo
  echo "Adding convenience aliases to ${BASHRC}..."
  {
    echo ""
    echo "# Oven cure cycle logging helpers"
    echo "alias plot_cure='python3 ${SCRIPT_DIR}/generate_graphs.py'"
    echo "alias cure_logs='cd ${LOG_DIR} && ls -lht'"
  } >> "${BASHRC}"
fi

# Optional desktop shortcut
if [ -d "${DESKTOP_DIR}" ]; then
  cat > "${DESKTOP_DIR}/Cure_Logs.desktop" << EOD
[Desktop Entry]
Type=Application
Name=Cure Logs
Comment=View powder coating cure cycle logs
Icon=folder
Exec=pcmanfm ${LOG_DIR}
Terminal=false
Categories=Utility;
EOD
  chmod +x "${DESKTOP_DIR}/Cure_Logs.desktop"
  echo "Created desktop shortcut at ${DESKTOP_DIR}/Cure_Logs.desktop"
fi

# Quick reference file
cat > "${LOG_DIR}/README.txt" << 'EOD'
OVEN CURE CYCLE DATA LOGGING
============================

This directory contains CSV files and graphs from automatic cure cycles.

FILE NAMING:
- cure_log_YYYYMMDD_HHMMSS.csv  - Raw temperature data
- cure_log_YYYYMMDD_HHMMSS.png  - Temperature graph (screen quality)
- cure_log_YYYYMMDD_HHMMSS.pdf  - Temperature graph (print quality)

GENERATING GRAPHS:
From Raspberry Pi terminal:
  plot_cure                    # Process all CSV files in this directory
  plot_cure <filename.csv>     # Process specific file

CSV COLUMNS:
- Time(s): Elapsed time in seconds from start
- CH1_Air(°C): Main air temperature sensor
- CH2(°C), CH3(°C), CH5(°C): Additional temperature sensors
- CH6_IR(°C): Infrared sensor (measures part temperature)
- Setpoint(°C): Target temperature for the cycle
- State: Current oven state (Warming/Ready/Curing/Complete)
EOD

echo
echo "=== Setup Complete ==="
echo
echo "Log directory: ${LOG_DIR}"
echo "Scripts directory: ${SCRIPT_DIR}"
echo
echo "USAGE:"
echo "  1. Run an auto cycle - data is logged automatically"
echo "  2. Generate graphs: plot_cure"
echo "  3. View logs: cure_logs"
echo
echo "Restart your terminal or run: source \"${BASHRC}\""
