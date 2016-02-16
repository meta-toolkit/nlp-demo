git submodule update
make -C cpp/build -j4
if [ $? -ne 0 ]; then
  echo "Compiling API server failed!"
  return 1
fi

make -C web/
if [ $? -ne 0 ]; then
  echo "Compiling coffeescript failed!"
  return 1
fi

cp web/javascript/index.js /srv/www/timan103/html/meta-nlp-demo/javascript/
cp web/index.html /srv/www/timan103/html/meta-nlp-demo/

sed -i 's/0.0.0.0:9001/timan103.cs.illinois.edu/' /srv/www/timan103/html/meta-nlp-demo/javascript/index.js
if [ $? -ne 0 ]; then
  echo "Replacing API URL failed!"
  return 1
fi

echo "Success! Don't forget to restart the API server if necessary."
