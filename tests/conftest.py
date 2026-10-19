import sys
import os

# Add scripts directory to path to allow importing the scripts
sys.path.insert(
    0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "scripts"))
)

collect_ignore_glob = ["../build_*/*"]
