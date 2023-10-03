import { useEffect, useState } from 'react';
import './App.css';
import Navigation from './components/Navigation/Navigation.tsx';
import { IoTAnalyticsClient, GetDatasetContentCommand } from '@aws-sdk/client-iotanalytics';
import axios from 'axios';
import Papa from 'papaparse'; 
import CircularProgress from '@mui/material/CircularProgress';
import Box from '@mui/material/Box';
import Header from './components/Header/Header.tsx';
import Footer from './components/Footer/Footer.tsx';
import { ThemeProvider } from '@mui/material/styles';
import CssBaseline from '@mui/material/CssBaseline';
import { useTheme } from './contexts/ThemeContext.tsx';
import { lightTheme, darkTheme } from './themes/theme.tsx';


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
  let idCounter = 0;
  return dataset.map((row) => ({
    id: (idCounter++).toString(),
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
  const [latestData, setLatestData] = useState<Data | null>(null);
  const { isDarkMode } = useTheme();

  const iotAnalyticsClient = new IoTAnalyticsClient({
    region: 'us-east-1',
    credentials: {
      accessKeyId: import.meta.env.VITE_ACCESS_KEY_ID,
      secretAccessKey: import.meta.env.VITE_SECRET_ACCESS_KEY,
    },
  });

  async function getDatasetContent() {
    const params = {
      datasetName: 'esp32_analytics_dataset',
    };

    try {
      const response = await iotAnalyticsClient.send(new GetDatasetContentCommand(params));
      const dataUri = response.entries && response.entries[0] && response.entries[0].dataURI;

      if (typeof dataUri === 'string') {
        const { data } = await axios.get(dataUri);

        Papa.parse(data, {
          header: true,
          complete: function (result) {
            if (result.data) {
              const filteredData = (result.data as Dataset[]).filter((row) => {
                const totalWeightValue = parseFloat(row.total_weight);
                return !isNaN(totalWeightValue) && totalWeightValue !== null;
              });
              const mappedData = mapDatasetToData(filteredData);
              const latestRow = mappedData.reduce((latest, current) => {
                return latest.dt > current.dt ? latest : current;
              });
              setLatestData(latestRow);
              setDatasetData(mappedData);
              setIsLoading(false);
            } else {
              console.error('Erro ao analisar o CSV: Nenhum dado encontrado.');
              setDatasetData([]);
              setIsLoading(false);
            }
          },
          error: function (error) {
            console.error('Erro ao analisar o CSV:', error);
            setDatasetData([]);
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
    setIsLoading(true);
    getDatasetContent();
  }, []);

  return (
    <>
      {!isLoading ? (
        <>
          <ThemeProvider theme={isDarkMode ? darkTheme : lightTheme}>
            <CssBaseline />
            <Header/>
            <Navigation
            datasetData={datasetData}
            latestData={latestData}
            />
            <Footer/>
          </ThemeProvider>
        </>
      ) : (
        <Box sx={{ 
          display: 'flex',
          justifyContent: 'center',
          alignItems: 'center',
          height: '100vh',
        }}>
          <CircularProgress />
        </Box>
      )}  
    </>
  );
}

export default App;
