import { useEffect, useState } from 'react';
import './App.css';
import CellTable from './components/CellTable/CellTable.tsx';
import CellConditions from './components/CellConditions/CellConditions.tsx';
import { IoTAnalyticsClient, GetDatasetContentCommand } from '@aws-sdk/client-iotanalytics';
import axios from 'axios';
import Papa from 'papaparse'; 

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

function App() {
  const [datasetData, setDatasetData] = useState<Dataset[]>([]);

  // Configurar as credenciais e o cliente AWS IoT Analytics
  const iotAnalyticsClient = new IoTAnalyticsClient({
    region: 'us-east-1',
    credentials: {
      accessKeyId: '',
      secretAccessKey: '',
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
              // Filtrar as linhas onde total_weight não é nulo
              const filteredData = (result.data as Dataset[]).filter((row) => row.total_weight !== '');
              setDatasetData(filteredData as Dataset[]); // Converta o resultado filtrado para Dataset[]
            } else {
              console.error('Erro ao analisar o CSV: Nenhum dado encontrado.');
              setDatasetData([]); // Defina como um array vazio em caso de erro
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
    getDatasetContent();
  }, []);

  return (
    <>
      <h1>Cell Checkup Table</h1>
      <CellTable/>
      <h2>Cell Health Conditions</h2>
      <CellConditions />
    </>
  );
}

export default App;
