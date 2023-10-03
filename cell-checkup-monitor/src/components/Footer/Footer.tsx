import Typography from '@mui/material/Typography';
import Link from '@mui/material/Link';

export default function Footer() {
    return(
        <Typography variant="body2" align="center" sx={{ width: '100%', margin: '10px auto', position: 'absolute', bottom: 0}}>
            {'Copyright © '}
            <Link color="inherit" href="https://www.toledobrasil.com">
             Toledo do Brasil Indústria de Balanças Ltda. 2023
            </Link>{'.'}
            <br />
            Todos os direitos reservados.
        </Typography>
    )
};
