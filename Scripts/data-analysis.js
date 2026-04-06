const fs = require('fs');

const data = fs.readFileSync('./inputs/pos_log.csv', { encoding: 'utf-8' }).split('\n');

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

const dmeUpdates = [];

data.forEach((entry, line) => {
  if (!entry.trim()) return;

  const fields = entry.split(';');
  if (fields.length < 6) return;

  const unit1 = fields[0];
  const unit2 = fields[1];
  const unit3 = fields[2];
  const mixed = fields[3];
  const real = fields[4];
  const dmeUpdating = ''; //fields[5].replace('\r', '');

  const parsePos = (pos) => {
    if (pos === '999,999') return null;
    const [lat, lng] = pos.split(',').map(parseFloat);
    return [lng, lat];
  };

  const u1 = parsePos(unit1);
  if (u1) unit1String.geometry.coordinates.push(u1);
  if (dmeUpdating === '1' && unit1String.geometry.coordinates.at(-2)) {
    dmeUpdates.push({
      type: 'Feature',
      properties: { name: `Unit 1 ${line}` },
      geometry: { type: 'Point', coordinates: unit1String.geometry.coordinates.at(-2) },
    });
  }

  const u2 = parsePos(unit2);
  if (u2) unit2String.geometry.coordinates.push(u2);
  if (dmeUpdating === '1' && unit2String.geometry.coordinates.at(-2)) {
    dmeUpdates.push({
      type: 'Feature',
      properties: { name: `Unit 2 ${line}` },
      geometry: { type: 'Point', coordinates: unit2String.geometry.coordinates.at(-2) },
    });
  }

  const u3 = parsePos(unit3);
  if (u3) unit3String.geometry.coordinates.push(u3);
  if (dmeUpdating === '1' && unit3String.geometry.coordinates.at(-2)) {
    dmeUpdates.push({
      type: 'Feature',
      properties: { name: `Unit 3 ${line}` },
      geometry: { type: 'Point', coordinates: unit3String.geometry.coordinates.at(-2) },
    });
  }

  const m = parsePos(mixed);
  if (m) mixedString.geometry.coordinates.push(m);
  if (dmeUpdating === '1' && mixedString.geometry.coordinates.at(-2)) {
    dmeUpdates.push({
      type: 'Feature',
      properties: { name: `Mixed ${line}` },
      geometry: { type: 'Point', coordinates: mixedString.geometry.coordinates.at(-2) },
    });
  }

  const r = parsePos(real);
  if (r) realString.geometry.coordinates.push(r);
  if (dmeUpdating === '1' && realString.geometry.coordinates.at(-2)) {
    dmeUpdates.push({
      type: 'Feature',
      properties: { name: `Real ${line}` },
      geometry: { type: 'Point', coordinates: realString.geometry.coordinates.at(-2) },
    });
  }
});

output.features.push(realString);
output.features.push(mixedString);
output.features.push(unit1String);
output.features.push(unit2String);
output.features.push(unit3String);
output.features.push(...dmeUpdates);

fs.writeFileSync('./outputs/pos_log.geojson', JSON.stringify(output), { encoding: 'utf-8' });
