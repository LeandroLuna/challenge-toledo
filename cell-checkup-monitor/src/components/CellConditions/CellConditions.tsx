import BatteryAlertIcon from '@mui/icons-material/BatteryAlert';
import BatteryFullIcon from '@mui/icons-material/BatteryFull';
import './CellConditions.css';

interface CellInfo {
  cellId: number;
  isCellBroken: boolean;
}

export default function CellConditions() {
  const cells: CellInfo[] = [
    { cellId: 1, isCellBroken: false },
    { cellId: 2, isCellBroken: true },
    { cellId: 3, isCellBroken: false },
    { cellId: 4, isCellBroken: true },
    { cellId: 5, isCellBroken: false },
    { cellId: 6, isCellBroken: true },
  ];

  return (
    <div className="battery-container">
      {cells.map((cell) => (
        <div key={cell.cellId} className="battery-icon">
          {cell.isCellBroken ? (
            <BatteryAlertIcon sx={{ fontSize: 80, color: 'red' }} />
          ) : (
            <BatteryFullIcon sx={{ fontSize: 80, color: 'lime' }} />
          )}
          <div className="cell-label">Cell {cell.cellId}</div>
        </div>
      ))}
    </div>
  );
}
