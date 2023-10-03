import SwipeableViews from 'react-swipeable-views';
import { useTheme } from '@mui/material/styles';
import AppBar from '@mui/material/AppBar';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import Box from '@mui/material/Box';
import React from 'react';
import CellTable from '../CellTable/CellTable';
import CellConditions from '../CellConditions/CellConditions';

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

interface TabPanelProps {
  children?: React.ReactNode;
  dir?: string;
  index: number;
  value: number;
}

interface NavigationProps {
  datasetData: Data[];
  latestData: Data | null;
}

function TabPanel(props: TabPanelProps) {
  const { children, value, index, ...other } = props;

  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`full-width-tabpanel-${index}`}
      aria-labelledby={`full-width-tab-${index}`}
      {...other}
    >
      {value === index && (
        <Box sx={{ p: 3 }}>
          {children}
        </Box>
      )}
    </div>
  );
}

function a11yProps(index: number) {
  return {
    id: `full-width-tab-${index}`,
    'aria-controls': `full-width-tabpanel-${index}`,
  };
}

export default function Navigation(props: NavigationProps) {
  const theme = useTheme();
  const [value, setValue] = React.useState(0);

  const handleChange = (_event: React.SyntheticEvent, newValue: number) => {
    setValue(newValue);
  };

  const handleChangeIndex = (index: number) => {
    setValue(index);
  };

  return (
    <Box sx={{ width: '100%', height: 'fit-content' }}>
      <AppBar position="static">
        <Tabs
          value={value}
          onChange={handleChange}
          textColor="inherit"
          variant="fullWidth"
          TabIndicatorProps={{ style: {backgroundColor: "#81d4fa"}} }
        >
          <Tab label="Tabela de checkup das células" {...a11yProps(0)} />
          <Tab label="Condições das células" {...a11yProps(1)} />
        </Tabs>
      </AppBar>
      <SwipeableViews
        axis={theme.direction === 'rtl' ? 'x-reverse' : 'x'}
        index={value}
        onChangeIndex={handleChangeIndex}
      >
        <TabPanel value={value} index={0} dir={theme.direction}>
          <CellTable
              numSelected={0}
              onRequestSort={() => {}}
              onSelectAllClick={() => {}}
              order="desc"
              orderBy="dt"
              rowCount={props.datasetData.length}
              data={props.datasetData}
            />
        </TabPanel>
        <TabPanel value={value} index={1} dir={theme.direction}>
          <CellConditions data={props.latestData}/>
        </TabPanel>
      </SwipeableViews>
    </Box>
  );
}