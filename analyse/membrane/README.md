# Structure of analyse.npy

Import **analysis.npy** with:
```python
import numpy as np
analysis = np.load("analysis.npy").item()
```

The `analysis` object is a `dict` where each key is the name of a sound file. Each `key` value contains the following data:

```python
analysis["sound1.wav"] = [peak_number, signal, frequency, damping]
```

To tune the analysis, one can tweak:

* Peak detection parameter
* Order of **ESPRIT**
* Signal initial shift amount (transient avoiding)
