## Usage

Those templates dependencies are maintained via [pnpm](https://pnpm.io) via `pnpm up -Lri`.

This is the reason you see a `pnpm-lock.yaml`. That being said, any package manager will work. This file can be safely be removed once you clone a template.

```bash
$ npm install # or pnpm install or yarn install
```

### Learn more on the [Solid Website](https://solidjs.com) and come chat with us on our [Discord](https://discord.com/invite/solidjs)

## Available Scripts

In the project directory, you can run:

### `npm run dev` or `npm start`

Runs the app in the development mode.<br>
Open [http://localhost:3000](http://localhost:3000) to view it in the browser.

The page will reload if you make edits.<br>

### `npm run build`

Builds the app for production to the `dist` folder.<br>
It correctly bundles Solid in production mode and optimizes the build for the best performance.

The build is minified and the filenames include the hashes.<br>
Your app is ready to be deployed!

## Deployment

You can deploy the `dist` folder to any static host provider (netlify, surge, now, etc.)

## Protobuf Generation 

### Win32
To generate the client-side Protobuf files, run this command in PowerShell from the project root,
not the client root:

~~~~Powershell
.\client\node_modules\.bin\protoc `
  --proto_path=client\src\protobuf `
  --plugin=protoc-gen-ts=.\client\node_modules\.bin\protoc-gen-ts.cmd `
  --ts_out=client\src\protobuf\gen `
  client\src\protobuf\*.proto
~~~~

## This project was created with the [Solid CLI](https://github.com/solidjs-community/solid-cli)
