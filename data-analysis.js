const fs = require("fs");

const data = fs.readFileSync("Examples/civaWin/out/Debug/data.csv", {encoding: "utf-8"}).split("\n");

const output = {
  "type": "FeatureCollection",
  "features": []
};

const unit1 = data.reduce((prev, entry) => {
  if (entry.split(";")[0] === "UNIT_1") prev.push(entry.split(";"));
  return prev;
}, []);
const unit2 = data.reduce((prev, entry) => {
  if (entry.split(";")[0] === "UNIT_2") prev.push(entry.split(";"));
  return prev;
}, []);
const unit3 = data.reduce((prev, entry) => {
  if (entry.split(";")[0] === "UNIT_3") prev.push(entry.split(";"));
  return prev;
}, []);

const mixedString = {
  "type": "Feature",
  "properties": {
    "name": "Mixed",
    "stroke-width": 2
  },
  "geometry": {
    "coordinates": [],
    "type": "LineString"
  }
}
const unit1String = {
  "type": "Feature",
  "properties": {
    "name": "Unit 1",
    "stroke": "red"
  },
  "geometry": {
    "coordinates": [],
    "type": "LineString"
  }
}
const realString = {
  "type": "Feature",
  "properties": {
    "name": "Real",
    "stroke": "magenta"
  },
  "geometry": {
    "coordinates": [],
    "type": "LineString"
  }
}
unit1.forEach(fields => {
  if (parseFloat(fields[4].split(",")[0]) === 999) return;

  realString.geometry.coordinates.push([
    parseFloat(fields[4].split(",")[1].trim()),
    parseFloat(fields[4].split(",")[0].trim())
  ]);

  if (parseFloat(fields[1].split(",")[1]) === 999) return;

  unit1String.geometry.coordinates.push([
    parseFloat(fields[2].split(",")[1]),
    parseFloat(fields[2].split(",")[0])
  ]);

  if (parseFloat(fields[3].split(",")[1]) === 999) return;

  mixedString.geometry.coordinates.push([
    parseFloat(fields[3].split(",")[1]),
    parseFloat(fields[3].split(",")[0])
  ]);
});
output.features.push(unit1String);
output.features.push(mixedString);
output.features.push(realString);

const unit2String = {
  "type": "Feature",
  "properties": {
    "name": "Unit 2",
    "stroke": "green"
  },
  "geometry": {
    "coordinates": [],
    "type": "LineString"
  }
}
unit2.forEach(fields => {
  if (parseFloat(fields[1].split(",")[1]) === 999) return;

  unit2String.geometry.coordinates.push([
    parseFloat(fields[2].split(",")[1]),
    parseFloat(fields[2].split(",")[0])
  ]);
});
output.features.push(unit2String);

const unit3String = {
  "type": "Feature",
  "properties": {
    "name": "Unit 3",
    "stroke": "blue"
  },
  "geometry": {
    "coordinates": [],
    "type": "LineString"
  }
}
unit3.forEach(fields => {
  if (parseFloat(fields[1].split(",")[1]) === 999) return;

  unit3String.geometry.coordinates.push([
    parseFloat(fields[2].split(",")[1]),
    parseFloat(fields[2].split(",")[0])
  ]);
});
output.features.push(unit3String);

fs.writeFileSync("position.json", JSON.stringify(output), {encoding: "utf-8"})