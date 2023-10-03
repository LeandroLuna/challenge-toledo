import { createTheme, Theme } from '@mui/material/styles';

interface ThemeConfig {
  lightTheme: Theme;
  darkTheme: Theme;
}

export const lightTheme = createTheme({
  palette: {
    mode: 'light',
    background: {
      default: '#01579b',
    },
  },
});

export const darkTheme = createTheme({
  palette: {
    mode: 'dark',
  },
});

export const themeConfig: ThemeConfig = {
  lightTheme,
  darkTheme,
};
