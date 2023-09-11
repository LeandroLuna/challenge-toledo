import './App.css'
import CellTable from './components/CellTable/CellTable.tsx'
import CellConditions from './components/CellConditions/CellConditions.tsx';

function App() {

  return (
    <>
      <h1>Cell Checkup Table</h1>
      <CellTable></CellTable>
      <h2>Cell Health Conditions</h2>
      <CellConditions></CellConditions>
    </>
  )
}

export default App;