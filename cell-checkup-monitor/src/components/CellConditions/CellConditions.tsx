import BatteryAlertIcon from '@mui/icons-material/BatteryAlert';
import BatteryFullIcon from '@mui/icons-material/BatteryFull';
import './CellConditions.css';

interface CellInfo {
  cellId: number;
  isCellBroken: boolean;
}

export type Data = {
  id: string;
  truckIdentifier: string;
  cell1: number;
  cell2: number;
  cell3: number;
  cell4: number;
  cell5: number;
  cell6: number;
  standardDeviation: number;
  totalWeight: number;
  dt: string; // Adicione o campo dt aqui
};

interface CellConditionsProps {
  data: Data | null; // Declare a prop 'data' do tipo Data ou null
}

export default function CellConditions({ data }: CellConditionsProps) {
  if (!data) {
    return null;
  }

  const cellValues = [data.cell1, data.cell2, data.cell3, data.cell4, data.cell5, data.cell6];

  const average = cellValues.reduce((acc, value) => acc + value, 0) / cellValues.length;

  const cells: CellInfo[] = [
    { cellId: 1, isCellBroken: false },
    { cellId: 2, isCellBroken: false },
    { cellId: 3, isCellBroken: false },
    { cellId: 4, isCellBroken: false },
    { cellId: 5, isCellBroken: false },
    { cellId: 6, isCellBroken: false },
  ];

  for (let i = 0; i < cellValues.length; i++) {
    const currentValue = cellValues[i];
    const otherValues = cellValues.slice(0, i).concat(cellValues.slice(i + 1));
    const maxDifference = Math.max(...otherValues) - Math.min(...otherValues);

    if (Math.abs(currentValue - average) > maxDifference) {
      cells[i].isCellBroken = true;
    }
  }

  return (
    <div className="battery-container">
      {cells.map((cell) => (
        <div key={cell.cellId} className="battery-icon">
          {cell.isCellBroken ? (
            <BatteryAlertIcon sx={{ fontSize: 80, color: 'red' }} />
          ) : (
            <BatteryFullIcon sx={{ fontSize: 80, color: 'lime' }} />
          )}
          <div className="cell-label-weight">Weight: {cellValues[cell.cellId - 1]}</div>
          <div className="cell-label-id">Cell {cell.cellId}</div>
        </div>
      ))}
    </div>
  );
}
