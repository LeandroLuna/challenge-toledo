import { useEffect, useState } from 'react';
import './App.css';
import CellTable from './components/CellTable/CellTable.tsx';
import CellConditions from './components/CellConditions/CellConditions.tsx';
import { IoTAnalyticsClient, GetDatasetContentCommand } from '@aws-sdk/client-iotanalytics';
import axios from 'axios';
import Papa from 'papaparse'; 
import CircularProgress from '@mui/material/CircularProgress';
import Box from '@mui/material/Box';

type Dataset = {
  __dt: string;
  cellpairs: string;
  standard_deviation: string;
  time: string;
  total_weight: string;
  truck_id: string;
  weight: string;
  weight_cell_1: string;
  weight_cell_2: string;
  weight_cell_3: string;
  weight_cell_4: string;
  weight_cell_5: string;
  weight_cell_6: string;
};

// Defina o tipo de dados que corresponde ao seu componente CellTable
type Data = {
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
  dt: string;
};

function mapDatasetToData(dataset: Dataset[]): Data[] {
  let idCounter = 0; // Inicializa um contador para criar IDs exclusivos
  return dataset.map((row) => ({
    id: (idCounter++).toString(), // Cria um ID único incremental
    truckIdentifier: row.truck_id,
    cell1: parseFloat(row.weight_cell_1),
    cell2: parseFloat(row.weight_cell_2),
    cell3: parseFloat(row.weight_cell_3),
    cell4: parseFloat(row.weight_cell_4),
    cell5: parseFloat(row.weight_cell_5),
    cell6: parseFloat(row.weight_cell_6),
    standardDeviation: parseFloat(row.standard_deviation),
    totalWeight: parseFloat(row.total_weight),
    dt: row.time,
  }));
}

function App() {
  const [datasetData, setDatasetData] = useState<Data[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const [latestData, setLatestData] = useState<Data | null>(null); // Para armazenar o dado mais recente

  // Configurar as credenciais e o cliente AWS IoT Analytics
  const iotAnalyticsClient = new IoTAnalyticsClient({
    region: 'us-east-1',
    credentials: {
      accessKeyId: import.meta.env.VITE_ACCESS_KEY_ID,
      secretAccessKey: import.meta.env.VITE_SECRET_ACCESS_KEY,
    },
  });

  // Função para obter o conteúdo do dataset
  async function getDatasetContent() {
    const params = {
      datasetName: 'esp32_analytics_dataset',
    };

    try {
      const response = await iotAnalyticsClient.send(new GetDatasetContentCommand(params));
      const dataUri = response.entries && response.entries[0] && response.entries[0].dataURI;

      if (typeof dataUri === 'string') {
        const { data } = await axios.get(dataUri);

        // Use o Papaparse para converter o CSV em JSON
        Papa.parse(data, {
          header: true,
          complete: function (result) {
            if (result.data) {
              // Filtrar as linhas onde total_weight não é nulo e fazer a conversão para Data
              const filteredData = (result.data as Dataset[]).filter((row) => {
                const totalWeightValue = parseFloat(row.total_weight);
                return !isNaN(totalWeightValue) && totalWeightValue !== null;
              });
              const mappedData = mapDatasetToData(filteredData);
              // Encontre o dado mais recente com base em row.dt
              const latestRow = mappedData.reduce((latest, current) => {
                return latest.dt > current.dt ? latest : current;
              });
              setLatestData(latestRow); // Define o dado mais recente
              setDatasetData(mappedData); // Define o resultado filtrado e convertido para Data[]
              setIsLoading(false); // Define como false após o carregamento bem-sucedido
            } else {
              console.error('Erro ao analisar o CSV: Nenhum dado encontrado.');
              setDatasetData([]); // Defina como um array vazio em caso de erro
              setIsLoading(false); // Define como false em caso de erro
            }
          },
          error: function (error) {
            console.error('Erro ao analisar o CSV:', error);
            setDatasetData([]); // Defina como um array vazio em caso de erro
          },
        });
      } else {
        console.error('Dados do Dataset não encontrados.');
      }
    } catch (error) {
      console.error('Erro ao acessar o Dataset:', error);
    }
  }

  useEffect(() => {
    setIsLoading(true); // Define como true enquanto os dados estão sendo carregados
    getDatasetContent();
  }, []);

  return (
    <>
      {!isLoading ? (
        <>
          <h1>Cell Checkup Table</h1>
          <CellTable
            numSelected={0}
            onRequestSort={() => {}}
            onSelectAllClick={() => {}}
            order="asc"
            orderBy="cell1"
            rowCount={datasetData.length}
            data={datasetData}
          />
            <h2>Cell Health Conditions</h2>
            <CellConditions data={latestData}/>
        </>
      ) : (
        <Box sx={{ 
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center', // Centralize verticalmente
          height: '100vh', // Defina uma altura para ocupar a tela inteira verticalmente
        }}>
          <CircularProgress />
        </Box>
      )}  
    </>
  );
}

export default App;
