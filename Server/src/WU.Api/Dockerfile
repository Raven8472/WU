FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
WORKDIR /src

COPY ["global.json", "./"]
COPY ["src/WU.Api/WU.Api.csproj", "src/WU.Api/"]
COPY ["src/WU.Application/WU.Application.csproj", "src/WU.Application/"]
COPY ["src/WU.Domain/WU.Domain.csproj", "src/WU.Domain/"]
COPY ["src/WU.Infrastructure/WU.Infrastructure.csproj", "src/WU.Infrastructure/"]

RUN dotnet restore "src/WU.Api/WU.Api.csproj"

COPY . .
RUN dotnet publish "src/WU.Api/WU.Api.csproj" -c Release -o /app/publish /p:UseAppHost=false

FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS runtime
WORKDIR /app
COPY --from=build /app/publish .
EXPOSE 8080
ENTRYPOINT ["dotnet", "WU.Api.dll"]
