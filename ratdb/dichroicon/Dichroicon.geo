{
  "name": "GEO",
  "index": "world",
  "run_range": [0, 0],
  "mother": "",
  "type": "sphere",
  "r_max": 20000.0,
  "material": "validwater",
  "invisible": 1,
}

{
  "name": "GEO",
  "index": "hexdichroicon",
  "mother": "world",
  "type": "hexdichroicon",
  "rs": [0, 200, 300, 500, 0],
  "zs": [-50, 0, 100, 500, 500],
  "panel_size": [5000, 5000, 500],
  "material": "validwater",
  "color": [1.0, 0.5, 0.0, 0.75],
  // "drawstyle": "wireframe"
}

//{
//  "name": "GEO",
//  "index": "detector",
//  "run_range": [0, 0],
//  "mother": "world",
//  "type": "sphere",
//  "r_max": 3500,
//  "material": "validwater",
//  "color": [1.0, 0.5, 0.0, 0.75]
//}

//////////////////
// PMTS
//////////////////
//{
//  "name": "GEO",
//  "index": "pmts",
//  "run_range": [0, 0],
//  "mother": "world",
//  "type": "pmtarray",
//  "pmt_model": "validPMT",
//  "pmt_detector_type": "idpmt",
//  "sensitive_detector": "/mydet/pmt/inner",
//  "pos_table": "PMTINFO_valid",
//  "orientation": "manual",
//}
