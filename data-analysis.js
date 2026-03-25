const fs = require('fs');

const data = fs.readFileSync('Examples/civaWin/out/Debug/pos_log.csv', { encoding: 'utf-8' }).split('\n');

const output = {
  type: 'FeatureCollection',
  features: [],
};

const unit1String = {
  type: 'Feature',
  properties: {
    name: 'Unit 1',
    stroke: 'red',
    'stroke-width': 1,
  },
  geometry: {
    coordinates: [],
    type: 'LineString',
  },
};

const unit2String = {
  type: 'Feature',
  properties: {
    name: 'Unit 2',
    stroke: 'green',
    'stroke-width': 1,
  },
  geometry: {
    coordinates: [],
    type: 'LineString',
  },
};

const unit3String = {
  type: 'Feature',
  properties: {
    name: 'Unit 3',
    stroke: 'blue',
    'stroke-width': 1,
  },
  geometry: {
    coordinates: [],
    type: 'LineString',
  },
};

const mixedString = {
  type: 'Feature',
  properties: {
    name: 'Mixed',
    stroke: 'orange',
    'stroke-width': 2,
  },
  geometry: {
    coordinates: [],
    type: 'LineString',
  },
};

const realString = {
  type: 'Feature',
  properties: {
    name: 'Real',
    stroke: 'magenta',
    'stroke-width': 4,
  },
  geometry: {
    coordinates: [],
    type: 'LineString',
  },
};

data.forEach((entry) => {
  if (!entry.trim()) return;

  const fields = entry.split(';');
  if (fields.length < 5) return;

  const unit1 = fields[0];
  const unit2 = fields[1];
  const unit3 = fields[2];
  const mixed = fields[3];
  const real = fields[4];

  const parsePos = (pos) => {
    if (pos === '999,999') return null;
    const [lat, lng] = pos.split(',').map(parseFloat);
    return [lng, lat];
  };

  const u1 = parsePos(unit1);
  if (u1) unit1String.geometry.coordinates.push(u1);

  const u2 = parsePos(unit2);
  if (u2) unit2String.geometry.coordinates.push(u2);

  const u3 = parsePos(unit3);
  if (u3) unit3String.geometry.coordinates.push(u3);

  const m = parsePos(mixed);
  if (m) mixedString.geometry.coordinates.push(m);

  const r = parsePos(real);
  if (r) realString.geometry.coordinates.push(r);
});

output.features.push(realString);
output.features.push(mixedString);
output.features.push(unit1String);
output.features.push(unit2String);
output.features.push(unit3String);

fs.writeFileSync('pos_log.geojson', JSON.stringify(output), { encoding: 'utf-8' });
