#!/bin/bash
set -e

# Список підмодулів, які треба оновити
submodules=(
  "external/IXWebSocket"
  "external/SimpleAmqpClient"
  "external/crow"
  "external/json"
  "external/jwt-cpp"
  "external/prometheus-cpp"
  "external/rabbitmq-c"
  "external/tracy"
  "prometheus-cpp"
)

for sub in "${submodules[@]}"; do
    if [ -d "$sub" ]; then
        echo "--------------------------------------"
        echo "Processing submodule: $sub"
        cd "$sub"

        # Скидання локальних змін
        git reset --hard
        git clean -fd

        # Отримати останні зміни з віддаленого репозиторію
        git fetch origin

        # Перевірити, яка основна гілка (main або master)
        if git show-ref --verify --quiet refs/heads/main; then
            branch="main"
        else
            branch="master"
        fi

        git checkout $branch
        git reset --hard origin/$branch

        cd - > /dev/null
        echo "$sub updated successfully."
    else
        echo "Submodule $sub not found!"
    fi
done

# Додати всі оновлені підмодулі до коміту
git add "${submodules[@]}"
echo "All submodules updated and staged for commit."
echo "Now you can commit with: git commit -m 'Update submodules to latest commits'"
